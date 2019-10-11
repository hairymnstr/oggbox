# path to the project from the directory containing the makefile
PROJ_DIR	= .
# path to the output folder from the project (e.g. if PROJ_DIR is DIRA and build is in the current folder this must be ..)
REL_PROJ_DIR	= .
# project name to start all files with
PROJ_NAME	= oggbox
# major/minor revision to append to the files.  You could replace this with code to retrieve version from a tag name...
MAJOR_VER	= 1
MINOR_VER	= 3

all:
	@echo "To properly set up the environment use the make_manufacturing_data.py script to execute this makefile"

TAG = $(MAJOR_VER)v$(MINOR_VER)
BUILD = $(PROJ_NAME)-$(TAG)
RELATIVE_BUILD = $(REL_PROJ_DIR)/$(BUILD)

setversion:
	$(Q)python3 $(PROJ_DIR)/library/scripts/set_temp_version.py --major=$(MAJOR_VER) --minor=$(MINOR_VER) $(PROJ_DIR)

restoreversion:
	$(Q)python3 $(PROJ_DIR)/library/scripts/set_temp_version.py --restore $(PROJ_DIR)

manufacturing: | $(BUILD)
	$(Q)python3 $(PROJ_DIR)/library/scripts/export_schematic.py $(DEBUG) --output-dir $(BUILD) --name-tag=-$(TAG) $(PROJ_DIR)/$(PROJ_NAME).pro
	$(Q)python3 $(PROJ_DIR)/library/scripts/export_bom.py $(DEBUG) --output-dir $(BUILD) --name-tag=-$(TAG) $(PROJ_DIR)/$(PROJ_NAME).pro
	$(Q)python3 $(PROJ_DIR)/library/scripts/export_gencad.py $(DEBUG) --output-dir $(BUILD) --name-tag=-$(TAG) $(PROJ_DIR)/$(PROJ_NAME).pro
	$(Q)python3 $(PROJ_DIR)/library/scripts/export_drill.py --output-dir $(BUILD) --use-auxiliary-axis-as-origin --name-tag=-$(TAG) $(PROJ_DIR)/$(PROJ_NAME).kicad_pcb
	# no NPTH holes in this design remove the redundant files
	$(Q)rm -f $(BUILD)/*DRILL_NPTH_MAP.pdf
	$(Q)rm -f $(BUILD)/*DRILL_NPTH.txt
	$(Q)python3 $(PROJ_DIR)/library/scripts/export_pos.py $(DEBUG) --output-dir $(BUILD) --name-tag=-$(TAG) $(PROJ_DIR)/$(PROJ_NAME).pro
	$(Q)python3 $(PROJ_DIR)/library/scripts/export_fabnotes.py $(DEBUG) --output-dir $(BUILD) --name-tag=-$(TAG) $(PROJ_DIR)/$(PROJ_NAME).pro
	$(Q)python3 $(PROJ_DIR)/library/scripts/plot.py --layers F.Cu --layers F.Paste --layers F.SilkS --layers F.Mask \
						       --layers B.Cu --layers B.Paste --layers B.SilkS --layers B.Mask \
						       --layers Edge.Cuts --layers Inner1.Cu --layers Inner2.Cu \
						       --exclude-edge-from-other-layers --plot-in-silk module-reference \
						       --plot-in-silk module-value \
						       --output-dir $(RELATIVE_BUILD) --use-auxiliary-axis-as-origin \
						       --subtract-soldermask-from-silk \
						       --name-tag=$(TAG) \
						       $(PROJ_DIR)/$(PROJ_NAME).kicad_pcb
	$(Q)python3 $(PROJ_DIR)/library/scripts/check_pos_files.py $(BUILD)
	$(Q)zip -r $(BUILD).zip $(BUILD)

$(BUILD):
	$(Q)mkdir $(BUILD)

clean:
	$(Q)rm -rf $(BUILD)
	$(Q)rm -f $(BUILD).zip
	$(Q)rm -f $(PROJ_DIR)/*.xml
	$(Q)rm -f $(PROJ_DIR)/*.pos
	$(Q)rm -f $(PROJ_DIR)/*.csv

