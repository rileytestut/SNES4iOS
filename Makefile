BUNDLE = SNES-HD.app
EMULATOR_SOURCE_DIR = src/snes4iphone_src
BINARY_FILE = $(EMULATOR_SOURCE_DIR)/snes4iphone
APP_BINARY = $(BUNDLE)/SNES-HD
PLIST_FILE = SNES4iPad-Info.plist
APP_PLIST = $(BUNDLE)/Info.plist

PACKAGE_DIR=package-dir
PACKAGE_CONTROL=package-control.txt
PACKAGE_FILE=SNES-HD.deb

RESOURCE_DIR = Resources

NIB_FILES = $(RESOURCE_DIR)/ControlPadConnectViewController.nib $(RESOURCE_DIR)/DetailView.nib $(RESOURCE_DIR)/MainWindow.nib $(RESOURCE_DIR)/SaveStateSelectionViewController.nib $(RESOURCE_DIR)/SettingsViewController.nib $(RESOURCE_DIR)/WebBrowserViewController.nib

RESOURCES = $(wildcard $(RESOURCE_DIR)/*.png) $(wildcard $(RESOURCE_DIR)/*.sh) $(NIB_FILES) $(RESOURCE_DIR)/snesadvance.dat

IBTOOL = ibtool
LDID = /usr/local/bin/ldid
DPKG_DEB = /opt/local/bin/dpkg-deb

SSH_DESTINATION = root@ipad:/Applications

all: bundle

binary:
	cd $(EMULATOR_SOURCE_DIR) && $(MAKE) all
	
nibs: $(NIB_FILES)
	
bundle: binary nibs
	mkdir -p $(BUNDLE)
	cp $(RESOURCES) $(BUNDLE)
	cp $(BINARY_FILE) $(APP_BINARY)
	cp $(PLIST_FILE) $(APP_PLIST)
	$(LDID) -S $(APP_BINARY)
	
package: bundle
	mkdir -p $(PACKAGE_DIR)/Applications
	mkdir -p $(PACKAGE_DIR)/DEBIAN
	mkdir -p "$(PACKAGE_DIR)/var/mobile/Media/ROMs/SNES/ROMs Go Here.txt"
	touch $(PACKAGE_DIR)/var/mobile/Media/ROMs/SNES/
	cp -r $(BUNDLE) $(PACKAGE_DIR)/Applications
	cp $(PACKAGE_CONTROL) $(PACKAGE_DIR)/DEBIAN/control
	export COPYFILE_DISABLE 
	export COPY_EXTENDED_ATTRIBUTES_DISABLE
	$(DPKG_DEB) -b $(PACKAGE_DIR) $(PACKAGE_FILE)

%.nib: %.xib
	$(IBTOOL) --compile $@ $<
	
transfer: bundle
	scp -r $(BUNDLE) $(SSH_DESTINATION)
	
clean:
	rm -rf $(BUNDLE)
	rm -rf $(PACKAGE_DIR)
	rm -rf $(NIB_FILES)
	rm -f $(PACKAGE_FILE)
	cd $(EMULATOR_SOURCE_DIR) && $(MAKE) clean