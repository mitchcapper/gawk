BEGIN {
	switch (q) {	# Comment 1
	case "a":	# Comment 2
		# comment 3
		a++
	case "b":
		# Comment 4
		break	# Comment 5
	default:	# Comment 6
		break	# Comment 7
	}		# Comment 8

	switch (b)	# Comment A
	{		# Comment B
	case "a":
		break;
	}
}
