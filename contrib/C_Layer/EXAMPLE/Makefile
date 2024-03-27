all: start boxlib imgExample kmenu libguic rasterworks done

start:
	@echo "Make C_layer examples"
	
boxlib:
	@echo "Make boxlib examples"
	$(MAKE) -C boxlib
	
imgExample:
	@echo "Make libimg examples"
	$(MAKE) -C img_example
	
kmenu:
	@echo "Make kmenu examples"
	$(MAKE) -C kmenu
	
libguic:
	@echo "Make libguic examples"
	$(MAKE) -C libguic_kolibri/make_boardmsg

rasterworks:
	@echo "Make Rasterworks examples"
	$(MAKE) -C rasterworks
	
done:
	@echo "-----------"
	@echo "|  Done!  |"
	@echo "-----------"