1:=   	0   	   	i
2:<   	i   	10   	4
3:jmp  	8
4:read   	   	   	a[i]
5:+   	i   	1   	T1
6:=   	T1   	   	i
7:jmp  	2
8:=   	0   	   	i
9:<   	i   	10   	11
10:jmp  	23
11:=   	i   	   	j
12:<   	j   	10   	14
13:jmp  	22
14:>   	a[i]   	a[j]   	16
15:jmp  	19
16:=   	a[i]   	   	temp
17:=   	a[j]   	   	a[i]
18:=   	temp   	   	a[j]
19:+   	j   	1   	T2
20:=   	T2   	   	j
21:jmp  	12
22:jmp  	9
23:=   	0   	   	i
24:<   	i   	10   	26
25:jmp  	30
26:write   	   	   	a[i]
27:+   	i   	1   	T3
28:=   	T3   	   	i
29:jmp  	24
