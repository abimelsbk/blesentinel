#!/usr/bin/env python3
"""Append an observed BLE test result to test-cases/test-matrix.csv.

This helper logs results only; it does not perform BLE operations.
"""
import argparse
import csv
from datetime import datetime, timezone
from pathlib import Path

VALID_RESULTS = {"NOT TESTED", "PASS", "FAIL", "INCONCLUSIVE"}

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--id", required=True, help="Test ID, e.g. B2")
    parser.add_argument("--result", required=True, choices=sorted(VALID_RESULTS))
    parser.add_argument("--actual", required=True, help="Observed behavior")
    parser.add_argument("--evidence", default="", help="Evidence path or packet reference")
    parser.add_argument("--notes", default="", help="Optional notes")
    args = parser.parse_args()

    repo_root = Path(__file__).resolve().parents[1]
    matrix = repo_root / "test-cases" / "test-matrix.csv"
    if not matrix.exists():
        raise SystemExit(f"Test matrix not found: {matrix}")

    rows = []
    with matrix.open("r", newline="", encoding="utf-8-sig") as f:
        reader = csv.DictReader(f)
        fieldnames = reader.fieldnames
        rows = list(reader)

    found = False
    for row in rows:
        if row["test_id"].strip().upper() == args.id.strip().upper():
            row["result"] = args.result
            row["actual_result"] = args.actual
            row["evidence"] = args.evidence
            row["notes"] = args.notes
            row["tested_at_utc"] = datetime.now(timezone.utc).isoformat(timespec="seconds")
            found = True
            break

    if not found:
        raise SystemExit(f"Unknown test ID: {args.id}. Add it to {matrix} first.")

    with matrix.open("w", newline="", encoding="utf-8") as f:
        writer = csv.DictWriter(f, fieldnames=fieldnames)
        writer.writeheader()
        writer.writerows(rows)

    print(f"Updated {args.id.upper()} in {matrix}")
    print("Confirm the entry reflects an actual observation before publishing.")

if __name__ == "__main__":
    main()
