out/%: %.c
	mkdir -p out
	gcc $*.c -o out/$*

.PHONY: run_% clean

run_%: out/%
	./out/$* $(ARGS)

clean:
	rm -rf out