#!/usr/bin/env python3
import os
import subprocess
import csv
from pathlib import Path

import concurrent.futures

CHILD_COUNTS = [10, 100, 1000, 10000, 100000]
ITERATIONS = 20
FORKBOMB_BIN = "./forkbomb"
ANALYZE_BIN = "./analyze_forkbomb"
OUTPUT_DIR = Path(".")
CSV_OUT = OUTPUT_DIR / "forkbomb_results.csv"

def run_forkbomb(count: int, iteration: int) -> Path:
  fname = OUTPUT_DIR / f"forkbomb_{count}_{iteration}.txt"
  try:
    p = subprocess.run(
      [FORKBOMB_BIN, "-c", str(count)],
      stdout=subprocess.PIPE,
      stderr=subprocess.STDOUT,
      text=True,
      check=False,
    )
    output = p.stdout or ""
  except Exception as e:
    output = f"ERROR_RUNNING_FORKBOMB: {e}\n"
  fname.write_text(output)
  return fname

def analyze_file(path: Path) -> str:
  try:
    p = subprocess.run([ANALYZE_BIN, str(path)], stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True, check=False)
    if p.returncode == 0:
      val = p.stdout.strip()
    else:
      # include stderr if nonzero
      val = (p.stdout + "\n" + p.stderr).strip()
  except Exception as e:
    val = f"ERROR_ANALYZE: {e}"
  # normalize to single-line for table/csv
  return " | ".join(line.strip() for line in val.splitlines() if line.strip()) or ""

def main():
  # 1) Run forkbomb tasks concurrently
  tasks = []
  for count in CHILD_COUNTS:
    for i in range(1, ITERATIONS + 1):
      tasks.append((count, i))

  print(f"Launching {len(tasks)} forkbomb runs...")
  generated_files = []
  max_workers = min(32, len(tasks))
  with concurrent.futures.ThreadPoolExecutor(max_workers=max_workers) as ex:
    futs = {ex.submit(run_forkbomb, count, i): (count, i) for count, i in tasks}
    for fut in concurrent.futures.as_completed(futs):
      count, i = futs[fut]
      try:
        path = fut.result()
        generated_files.append((count, i, path))
        print(f"Finished forkbomb -c {count} iteration {i} -> {path}")
      except Exception as e:
        print(f"Error running forkbomb -c {count} iteration {i}: {e}")

  # 2) Analyze each generated file and collect results
  print("Analyzing files...")
  results = {}  # (iteration, count) -> value
  # Keep ordering predictable: iterate count outer and iteration inner or vice versa. We'll fill by iteration rows.
  for count in CHILD_COUNTS:
    for i in range(1, ITERATIONS + 1):
      path = OUTPUT_DIR / f"forkbomb_{count}_{i}.txt"
      if not path.exists():
        val = "MISSING_FILE"
      else:
        val = analyze_file(path)
      results[(i, count)] = val
      print(f"Analyzed {path}: {val}")

  # 3) Print pretty table (iterations as rows, counts as columns)
  # Build headers and rows
  headers = ["iter"] + [str(c) for c in CHILD_COUNTS]
  rows = []
  for i in range(1, ITERATIONS + 1):
    row = [str(i)]
    for c in CHILD_COUNTS:
      row.append(results.get((i, c), ""))
    rows.append(row)

  # Compute column widths
  cols = list(zip(*([headers] + rows)))
  col_widths = [max(len(cell) for cell in col) for col in cols]

  # Print table
  def format_row(row):
    return " | ".join(cell.ljust(col_widths[idx]) for idx, cell in enumerate(row))

  sep = "-+-".join("-" * w for w in col_widths)
  print("\nResults table:")
  print(format_row(headers))
  print(sep)
  for row in rows:
    print(format_row(row))

  # 4) Output CSV
  with CSV_OUT.open("w", newline="") as csvfile:
    writer = csv.writer(csvfile)
    writer.writerow(headers)
    for row in rows:
      writer.writerow(row)

  print(f"\nCSV written to {CSV_OUT}")
  # Also print CSV to stdout
  print("\nCSV output:")
  with CSV_OUT.open("r") as f:
    print(f.read())

if __name__ == "__main__":
  main()