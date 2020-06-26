import argparse
from pathlib import Path
from shutil import rmtree, copy
import json
import sys
from typing import Sequence
from subprocess import check_call as run_proc
from contextlib import contextmanager, ExitStack


def ensure_absent(path: Path) -> None:
    if path.is_dir():
        rmtree(path)
    elif path.exists():
        path.unlink()


@contextmanager
def tentative_remove(path: Path):
    tmp = path.parent / (path.name + '.bak')
    ensure_absent(tmp)
    try:
        if path.exists():
            path.rename(tmp)
        yield
    except:
        # An exception? Restore the files
        ensure_absent(path)
        if tmp.exists():
            tmp.rename(path)
        raise
    else:
        ensure_absent(tmp)


@contextmanager
def remove_later(path: Path):
    try:
        yield
    finally:
        ensure_absent(path)


def main(argv: Sequence[str]) -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument('revision', help='The revision to check out')
    args = parser.parse_args(argv)

    root = Path(__file__).resolve().parent

    src_dir = root / 'src'
    inc_dir = root / 'include'

    run_proc(['git', 'reset'])

    with ExitStack() as ctx:
        ctx.enter_context(tentative_remove(src_dir))
        run_proc(['git', 'checkout', args.revision, '--', 'src'])

        # Move the src/ directory from upstream to another location. We want to
        # replace it.
        upstream_root = root / 'upstream'
        ctx.enter_context(remove_later(upstream_root))
        src_dir.rename(upstream_root)
        # Move the inner src/libsodium up to just src/
        (upstream_root / 'libsodium').rename(src_dir)

        # Remove Makefile.am
        for p in src_dir.rglob('Makefile.am'):
            p.unlink()

        next(iter(src_dir.rglob('version.h.in'))).unlink()

        # Extract the include/ directory from the src/ directory
        ctx.enter_context(tentative_remove(inc_dir))
        inner_include = src_dir / 'include'
        inner_include.rename(inc_dir)

        # We have a generated version.h file, but there is a committed copy we can just use
        run_proc(
            ['git', 'checkout', args.revision, '--', 'builds/msvc/version.h'])
        (root / 'builds/msvc/version.h').rename(inc_dir / 'sodium/version.h')
        ensure_absent(root / 'builds')
        common_h = inc_dir / 'sodium/private/common.h'
        common_h.write_text((root / 'config.h').read_text() + '\n' +
                            common_h.read_text())

        # The .c files expect to be able to resolve headers with unqualified paths.
        # This is annoying, but we can hack by copying the header files to the appropriate
        # paths.
        inc_sodium_dir = inc_dir / 'sodium'
        for header in inc_sodium_dir.rglob('*.h'):
            relpath = header.relative_to(inc_sodium_dir)
            dest = src_dir / relpath
            dest.parent.mkdir(exist_ok=True, parents=True)
            copy(header, dest)

        # the private/ directory in include/ should not be part of the sdist
        rmtree(inc_dir / 'sodium/private')

        (root / 'package.jsonc').write_text(
            json.dumps(
                {
                    'name': 'libsodium',
                    'namespace': 'libsodium',
                    'version': args.revision,
                },
                sort_keys=True,
                indent=2,
            ))

        run_proc(['git', 'reset'])
        run_proc(['git', 'add', '--', 'include', 'src'])

    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
