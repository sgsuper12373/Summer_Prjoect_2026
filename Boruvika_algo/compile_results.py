#!/usr/bin/env python3
"""Summarize Boruvka benchmark CSVs into a markdown report.

Reads every Results/<testfile>_result.csv produced by ECL_boruvkas, and for
each test writes the median run time of every version plus its speedup
relative to the serial_full version (speedup = median(serial_full) / median(v)).
The raw per-run timings are NOT copied into the report.
"""

import csv
import glob
import os
from statistics import median

RESULTS_DIR = "Results"
OUT_FILE = os.path.join(RESULTS_DIR, "Results.MD")
BASELINE = "serial_full"


def summarize_csv(path):
    """Return (versions, medians dict, weight) for one result CSV."""
    with open(path, newline="") as fh:
        rows = list(csv.reader(fh))
    header = rows[0]
    versions = header[2:]                      # columns after run_no, weight
    cols = {v: [] for v in versions}
    weight = None
    for r in rows[1:]:
        if not r:
            continue
        weight = r[1]
        for v, val in zip(versions, r[2:]):
            cols[v].append(int(val))
    medians = {v: median(vals) for v, vals in cols.items()}
    return versions, medians, weight


def main():
    files = sorted(glob.glob(os.path.join(RESULTS_DIR, "*_result.csv")))
    lines = ["# Boruvka MST Benchmark Results", ""]
    lines.append(f"Speedup is relative to `{BASELINE}`, computed from the "
                 "median run time over all runs (time in microseconds).")
    lines.append("")

    for path in files:
        name = os.path.basename(path)[:-len("_result.csv")]
        versions, medians, weight = summarize_csv(path)
        base = medians.get(BASELINE)

        lines.append(f"## {name}")
        lines.append("")
        if weight is not None:
            lines.append(f"MST weight: `{weight}`")
            lines.append("")
        lines.append("| Version | Median time (µs) | Speedup vs serial_full |")
        lines.append("| --- | ---: | ---: |")
        for v in versions:
            m = medians[v]
            speedup = base / m if (base and m) else float("nan")
            lines.append(f"| {v} | {m:,.0f} | {speedup:.2f}× |")
        lines.append("")

    with open(OUT_FILE, "w") as fh:
        fh.write("\n".join(lines) + "\n")
    print(f"Wrote summary for {len(files)} test file(s) to {OUT_FILE}")


if __name__ == "__main__":
    main()
