#!/usr/bin/env python3
########################################################################
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
########################################################################

import argparse
import csv
from pathlib import Path


def _reflow_text(text, width=70):
    tokens = text.split()
    lines = ["",]
    for token in tokens:
        this_line = lines[-1]
        if this_line:
            this_line += " " + token
        else:
            this_line = token

        if len(this_line) > width:
            lines.append(token)
        else:
            lines[-1] = this_line

    return '\n'.join(lines)


def _pad_right(text, width, pad_char=' '):
    gap = width - len(text)
    if gap < 0:
        return
    text += pad_char * gap
    return text


def _draw_graph(records):
    min_v = min([x[1] for x in records])
    max_v = max([x[1] for x in records])
    n_ticks_max = 55
    tick_width = max_v / n_ticks_max

    lines = list()
    for record in records:
        n_ticks = int(record[1] / tick_width)
        label = f"({record[0][0]}, {record[0][1]})"
        bar = 'o' * n_ticks
        bar += f" {record[1]:.4f}"
        lines.append([label, bar])

    # Pad the labels to make their widths equal.
    max_label_width = max([len(line[0]) for line in lines])
    for line in lines:
        line[0] = _pad_right(line[0], max_label_width)

    # Print the top graph header and axis
    line = _pad_right(" Category", max_label_width + 1)
    line += "| Average duration (seconds)"
    print(line)

    line = '-' * max_label_width
    line += "-+-" + '-' * n_ticks_max + '>'
    print(line)

    for line in lines:
        line = f"{line[0]} | {line[1]}"
        print(line)

    print()

    top = records[0]
    text = f"Storage of {top[0][0]} with the LU factor of {top[0][1]} appears to be the best choice in this environment."
    print(_reflow_text(text))


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("input", type=Path, help="Input CSV file containing raw benchmark data.")
    args = parser.parse_args()

    totals = dict()
    with open(args.input, 'r') as f:
        reader = csv.DictReader(f, delimiter=',')
        for row in reader:
            label = (row['storage'], row['factor'])
            duration = float(row['duration'])
            if label in totals:
                totals[label][0] += duration
                totals[label][1] += 1
            else:
                totals[label] = [duration, 1]

    totals = [(k, v) for k, v in totals.items()]
    averages = [(k, t / n) for k, (t, n) in totals]

    averages = sorted(averages, key=lambda x: x[1])
    _draw_graph(averages)


if __name__ == "__main__":
    main()

