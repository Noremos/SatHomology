from datetime import datetime

# Read the tests.txt
with open('tests.txt') as f:
	lines = f.readlines()
	# Parse the timestamps
	timestamps = [(datetime.strptime(line.split(" ")[1], '%H:%M:%S.%f'),line.split(" ")[2])  for line in lines]
	itmes = [line.split(" ")[3] for line in lines]

	# Calculate the execution times (differences between consecutive timestamps)
	execution_times = [((timestamps[i+1][0] - timestamps[i][0]).total_seconds(),timestamps[i][1]) for i in range(len(timestamps) - 1)]

	# Sort the execution times in descending order
	execution_times.sort(reverse=True, key=lambda x: x[0])

	# Print the top 30 longest execution times
	for time in execution_times[:30]:
		print(time)