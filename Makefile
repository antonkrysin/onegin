obj = onegin.o

onegin : $(obj)
	cc -o onegin $(obj)

onegin.o :

.PHONY : clean
clean:
	rm onegin $(obj)
