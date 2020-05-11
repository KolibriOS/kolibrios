def test(x):
	f = ['0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F']
	txt = ""
	while x:
		txt = f[x&0xF]+txt
		x = x>>4
	
	return txt
	
stdout(test(0x123454))