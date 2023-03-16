out/%: %.c
	mkdir -p out
	gcc $*.c -o out/$*

.PHONY: run/% clean

run/%: out/%
	./out/$*

clean:
	rm -rf out