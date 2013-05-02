dcl 1 guripa_cmd	based,
      3 code		char(4),
      3 rest		union,
	5 spaces        char(124),
        5 values,
	  7 gurpid	bin fixed(31),
	  7 interval	bin float(24),
	  7 maxidle	bin fixed(15),
	  7 msgidle	bin fixed(15);


