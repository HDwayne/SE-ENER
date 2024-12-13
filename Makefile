APP_ANALYSE_DIR=app_analyse
FRACTAL_DIR=fractal

.PHONY: all clean test

all:
	$(MAKE) -C $(APP_ANALYSE_DIR)
	$(MAKE) -C $(FRACTAL_DIR)

clean:
	$(MAKE) -C $(APP_ANALYSE_DIR) clean
	$(MAKE) -C $(FRACTAL_DIR) clean

test:
	$(MAKE) -C $(APP_ANALYSE_DIR) test NAME="$(NAME)"
