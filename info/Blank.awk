# Will Edgington, wedgingt@nike.cair.du.edu
/^$/    { if (prev != "") {
		print
		prev = ""
		}
	next }
        {
		print
		prev = "a"
	}
