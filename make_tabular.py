#!/usr/bin/env python3
import sys

endl = "\\\\"

nCols = 0
rows = []
nAgents = 0

for line in open(sys.argv[1], 'r').readlines():
	nAgents += 1
	row = line.split()
	rows.append(row)
	nCols = max(nCols, len(row))

table = []
for row in rows:
	for _ in range(nCols - len(row)):
		#row.append(row[-1])
                row.append('\\textbf{S}')
	table.append(' & '.join(row))

tableSpec = ' l || ' + (' l | ' * nCols)
tableSpec = tableSpec[:-2]

timesteps = ' & '.join(list(map(str, range(nCols))))

print(f"\\begin{{tabular}}{{{tableSpec}}}")
print(f"Timestep & {timesteps} {endl}")
print("\\hline")
for agent in range(nAgents):
	print(f"Agent {agent} & {table[agent]} {endl}")
print(f"\\end{{tabular}}")
