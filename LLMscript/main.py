#!/usr/bin/env python3
"""
Recursively delete:
  - All files with given extensions (default: .svg, .md)
  - All directories with given names (default: docs, imgs)

Usage:
  python3 clean.py /path/to/target \
      --extensions .svg,.md \
      --dirnames docs,imgs

Time complexity: O(N) where N = total files + directories under target.
Memory complexity: O(1) extra space (ignoring recursion/iterator overhead).
"""

import argparse
import sys
import os                # ← 여기에 추가
import shutil
from pathlib import Path


def clean_tree(
    root: Path,
    extensions: set[str],
    dirnames_to_remove: set[str]
) -> None:
    """
    Walk through `root` top-down, deleting matching files and directories.

    Args:
        root: Path to the root directory to clean.
        extensions: Set of lowercase file extensions to delete (including dot).
        dirnames_to_remove: Set of directory names; any directory with name in
                            this set will be removed entirely.
    Raises:
        FileNotFoundError: If `root` does not exist.
        NotADirectoryError: If `root` is not a directory.
    """
    if not root.exists():
        raise FileNotFoundError(f"Directory not found: {root}")
    if not root.is_dir():
        raise NotADirectoryError(f"Not a directory: {root}")

    # os.walk를 사용할 수 있게 됨
    for current_dir, dirnames, filenames in os.walk(root, topdown=True):
        # 먼저 지울 디렉터리 제거
        for dirname in list(dirnames):
            if dirname in dirnames_to_remove:
                dirpath = Path(current_dir) / dirname
                try:
                    shutil.rmtree(dirpath)
                    print(f"Removed directory: {dirpath}")
                except PermissionError:
                    print(f"Permission denied (dir): {dirpath}", file=sys.stderr)
                except OSError as err:
                    print(f"Error removing {dirpath}: {err}", file=sys.stderr)
                dirnames.remove(dirname)  # 하위 순회 방지

        # 그 다음, 파일 삭제
        for fname in filenames:
            fpath = Path(current_dir) / fname
            if fpath.suffix.lower() in extensions:
                try:
                    fpath.unlink()
                    print(f"Deleted file: {fpath}")
                except PermissionError:
                    print(f"Permission denied (file): {fpath}", file=sys.stderr)
                except OSError as err:
                    print(f"Error deleting {fpath}: {err}", file=sys.stderr)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Recursively delete files by extension and directories by name."
    )
    parser.add_argument(
        "root",
        type=Path,
        nargs="?",
        default=Path.cwd(),
        help="Target directory (default: current working dir)."
    )
    parser.add_argument(
        "--extensions",
        type=lambda s: { (e if e.startswith('.') else '.'+e).lower() for e in s.split(',') },
        default={'.svg', '.md'},
        help="Comma-separated file extensions to delete (default: .svg,.md)."
    )
    parser.add_argument(
        "--dirnames",
        type=lambda s: { e.strip() for e in s.split(',') },
        default={'docs', 'imgs'},
        help="Comma-separated directory names to remove (default: docs,imgs)."
    )
    return parser.parse_args()


def main() -> None:
    args = parse_args()
    try:
        clean_tree(args.root, args.extensions, args.dirnames)
    except (FileNotFoundError, NotADirectoryError) as err:
        print(f"Error: {err}", file=sys.stderr)
        sys.exit(1)


if __name__ == "__main__":
    main()
