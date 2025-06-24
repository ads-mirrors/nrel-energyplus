import argparse
import re
from collections import Counter
from pathlib import Path

RE_FULLNAME = re.compile(r"<!-- FullName:(?P<activeReportName>[^_]+)_(?P<activeForName>[^_]+)_(?P<subtitle>[^_]+)-->")


def format_row(row, col_widths):
    return "| " + " | ".join(f"{str(cell):<{col_widths[i]}}" for i, cell in enumerate(row)) + " |"


def ensure_unique_html_tables(html_path: Path):
    """
    Ensure that each HTML table in the report has a unique name.
    If duplicates are found, append a number to the duplicate names.
    """
    content = html_path.read_text()

    matches = RE_FULLNAME.findall(content)
    if not matches:
        raise ValueError("No HTML tables found with FullName pattern")

    # Filter only duplicates (count > 1)
    duplicates = [
        (activeReportName, activeForName, subtitle, count)
        for (activeReportName, activeForName, subtitle), count in Counter(matches).items()
        if count > 1
    ]

    # Include header row in width calculation
    if duplicates:
        print(f"Found {len(duplicates)} duplicate HTML table(s) based on FullName\n")
        header = [("activeReportName", "activeForName", "subtitle", "count")]
        rows = header + duplicates
        # Compute max width for each column
        col_widths = [max(len(str(row[i])) for row in rows) for i in range(len(header[0]))]

        print(format_row(rows[0], col_widths))
        print("|-" + "-|-".join("-" * w for w in col_widths) + "-|")
        for row in duplicates:
            print(format_row(row, col_widths))

        exit(1)
    else:
        print("No duplicates found.")
        exit(0)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Compare Output Reports.")
    parser.add_argument("out_dir", type=Path, help="Output directory where to find the reports")
    parser.add_argument(
        "--skip-missing", action="store_true", default=False, help="Do not raise if the eplustbl.htm isn't found"
    )
    args = parser.parse_args()

    out_dir = args.out_dir.resolve()
    if not (out_dir.exists() and out_dir.is_dir()):
        raise IOError(f"'{out_dir}' is not a valid directory")
    html_path = out_dir / "eplustbl.htm"
    if not html_path.exists():
        if args.skip_missing:
            print(f"Skipping missing HTML file: {html_path}")
            exit(0)
        raise IOError(f"HTML '{html_path}' does not exist")

    ensure_unique_html_tables(html_path=html_path)
