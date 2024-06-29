import random
seen_values = set()
result = 0
miv = 100
mav = 0
stvr = 0

arr = [23 , 21 , 24 , 27 , 22 , 27 , 26 , 25, 21,1,1, 19 ]
for i in range(len(arr)):
	a = random.randint(1, 100)
	a = arr[i]
	if i == 0:
		miv = a
		mav = a
		stvr = mav


	if a > mav:
		for j in range(mav + 1, a):
			stvr ^= j

		mav = a
	if a < miv:
		for j in range(a, mav - 1):
			stvr ^= j
		miv = a


	# Check if the value has been seen before

	stvr = 0
	if miv != mav:
		for j in range(miv, mav + 1):
			stvr ^= j
		stvr = miv ^ mav

	resultPrev = result
	result ^= a

	r1 = result ^ stvr
	r2 = resultPrev ^ stvr

	print(f"a:{a}, {r1}<=>{r2}")
	if a in seen_values:


		# 	mav = a
		# if a < miv:
		# 	for j in range(a, mav - 1):
		# 		stvr ^= j
		# 	miv = a
		# If the value is a duplicate, handle it here
		# if result ^ stvr != a:
		# 	print("NOOOO")

		print("Duplicate value detected:", a)
		# You can choose to skip further processing if it's a duplicate
		# continue
	else:
		# If it's not a duplicate, add it to the set of seen values
		seen_values.add(a)
		# Proceed with further processing of 'a'

	# print(a, end= ' ')  # Output the current value
