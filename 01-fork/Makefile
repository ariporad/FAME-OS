out/%: %.c
	mkdir -p out
	gcc $*.c -o out/$*

.PHONY: run_% clean

run_%: out/%
	./out/$*

clean:
	rm -rf out