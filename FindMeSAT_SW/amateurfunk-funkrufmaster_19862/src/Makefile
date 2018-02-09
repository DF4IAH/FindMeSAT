###############################################################################
#                   Makefile fuer das Funkruf-Projekt                         #
#                       Stand: 22.07.2004                                     #
###############################################################################
# Alle Installationsabhaengigen Parameter stehen nun in der
# Datei CONFIGURATION. Damit braucht fuer 
# Rechnerspezifische Anpassungen nur noch die Datei
# CONFIGURATION, nicht aber mehr das Makefile selbst
# editiert werden.

# include CONFIGURATION

ifeq (.config,$(wildcard .config))
include .config
do-it-all:	frmaster
else
CONFIGURATION = config
do-it-all:	config
endif


CONFIG_SHELL := $(shell if [ -x "$$BASH" ]; then echo $$BASH; \
	  else if [ -x /bin/bash ]; then echo /bin/bash; \
	  else echo sh; fi ; fi)

# Compiler
CPP=g++
# Verzeichnisse
CONF_DIR=$(BASE_DIR)etc/
DIGI_DIR=$(CONF_DIR)digis/
BIN_DIR=$(BASE_DIR)bin/
LIB_DIR=$(BASE_DIR)lib/
HTML_DIR=$(BASE_DIR)lib/html/
FWD_DIR=$(BASE_DIR)fwd/
SPOOL_DIR=$(BASE_DIR)spool/
GRUPPEN_DIR=$(LIB_DIR)gruppen/
RUBRIKEN_DIR=$(LIB_DIR)rubriken/
LOG_DIR=$(BASE_DIR)log/
IMPORT_DIR=$(BASE_DIR)import/
EXPORT_DIR=$(BASE_DIR)export/

# Compiler-Flags

ifeq ($(COMPILE_DEBUG),JA)
DEBUGFLAGS=-g -D_DEBUG_
OPTIMIZE_FLAG=-O0
else
DEBUGFLAGS=
OPTIMIZE_FLAG=-O3
endif


CFLAGS=$(OPTIMIZE_FLAG) -Iinclude -Wall $(DEBUGFLAGS) $(SUSE8)
ifeq ($(CONFIG_GCC_3),JA)
CFLAGS += -Wno-deprecated
endif

# Benoetigte Libraries

ifeq ($(COMPILE_AX25),JA)
AX25LIB = /usr/lib/libax25.a
endif

ifeq ($(COMPILE_SLAVES),JA)

ifeq ($(CONFIG_PPC),JA)	
SPEZLIB = spezlib.ppc
else

ifeq ($(CONFIG_EGCS_291),JA)
SPEZLIB = spezlib.386.egcs291
else
ifeq ($(CONFIG_GCC_295),JA)
SPEZLIB = spezlib.386.gcc295
else
ifeq ($(CONFIG_GCC_3),JA)
SPEZLIB = spezlib.386.gcc320
else
SPEZLIB = spezlib
endif
endif
endif
endif

endif

LIBS = $(AX25LIB) $(SPEZLIB) -lcrypt

# Dateien
FRMCONF=texte/frmaster.conf
SLAVECONF=texte/slave.conf
FWDCONF=texte/fwd.conf
PASSWORD=texte/passwd
STARTSCRIPT=texte/start_frmaster

# Ab hier DARF nichts mehr geaendert zu werden!!!!
#-------------------------------------------------------------------------------
.cpp.o:
	@echo Compiling $<
	@$(CPP) $(CFLAGS) -c $<

TOBJ = adress.o \
       bake.o board.o \
       callsign.o connector.o connect_interface.o \
       cmdline.o connect_string.o config.o crontab.o \
       database.o destin.o default.o\
       export.o \
       fwd_protokoll.o fwd_nachrichten.o fwd_descriptoren.o fwd_router.o\
       fwd_frontend.o fwd_interface.o fwd_execution.o fwd_autorouter.o\
       gruppe.o \
       iputil.o import.o interfaces.o \
       logfile.o locator.o lang_support.o\
       makros.o my_hash_map.o mid.o make_boards.o \
       pager.o passwd.o\
       recv_select.o relais.o \
       schedule.o schedule_incoming.o schedule_outgoing.o schedule_servers.o\
       schedule_process.o statistics.o\
       socket.o String.o system_info.o \
       talk.o texte.o \
       thread.o trace.o \
       user.o user_interface.o \
       user_interface_admin.o user_interface_boards.o user_interface_dbase.o \
       user_interface_fwd.o user_interface_group.o user_interface_infos.o \
       user_interface_page.o \
       zeit.o

ifeq ($(COMPILE_AX25),JA)
OBJ_AX25  = ax25.o
endif
ifeq ($(COMPILE_CLUSTER),JA)
OBJ_CLUSTER  = cluster_interface.o cluster.o
endif
ifeq ($(COMPILE_TCP),JA)
OBJ_TCP  = tcp.o
endif
ifeq ($(COMPILE_SAT),JA)
OBJ_SAT = sat.o
endif
ifeq ($(COMPILE_WX),JA)
OBJ_WX = wx_interface.o wx.o user_interface_wx.o
endif
ifeq ($(COMPILE_ASTRO),JA)
OBJ_ASTRO = astronom.o
endif
ifeq ($(COMPILE_TIDE),JA)
OBJ_TIDE = gezeiten.o
endif
ifeq ($(COMPILE_SLAVES),JA)
OBJ_SLAVES = rpc_interface.o slaves.o spoolfiles.o user_interface_slaves.o
endif
ifeq ($(COMPILE_DIGISTATUS),JA)
OBJ_DIGI = digi.o user_interface_digi.o
endif
ifeq ($(COMPILE_SMTP),JA)
OBJ_SMTP = smtp_interface.o
endif
ifeq ($(COMPILE_HTTP),JA)
OBJ_HTTP = http_interface.o
endif
ifeq ($(COMPILE_AXHTP),JA)
OBJ_AXHTP = axhtp_interface.o
endif
ifeq ($(COMPILE_HTML),JA)
OBJ_HTML = html_objects.o html_align.o html_frames.o html_forms.o\
           html_references.o html_tabular.o html_paartags.o \
           html_generator.o html_generator_dir.o html_generator_user.o
endif
POBJ = $(OBJ_AX25) $(OBJ_ASTRO)  $(OBJ_CLUSTER) $(OBJ_TIDE)  $(OBJ_TCP)  \
       $(OBJ_SAT)  $(OBJ_SLAVES) $(OBJ_WX)      $(TOBJ)      $(OBJ_SMTP) \
       $(OBJ_HTTP) $(OBJ_AXHTP)  $(OBJ_HTML)    $(OBJ_DIGI)


all:	      do-it-all


config:
	@$(CONFIG_SHELL) scripts/Configure scripts/config.in

base_install: install_conf


frmaster: $(CONFIGURATION) $(POBJ)
	@echo Linking $<
	$(CPP) -o frmaster $(DEBUGFLAGS) $(POBJ) $(LIBS)

clean:
	@rm -f frmaster make_boards *.o *~ *.bak include/*~ scripts/*~
	@rm -f .#* include/.#* scripts/.#*
	@rm -f texte/*~ texte/.#*

install: all
	install -d -m 755 $(BASE_DIR)
	install -d -m 755 $(BIN_DIR)
	install -d -m 755 $(LIB_DIR)
	install -d -m 755 $(HTML_DIR)
	install -d -m 755 $(DIGI_DIR)
	install -m 755 frmaster $(BIN_DIR)frmaster
	install -m 644 texte/help.dl $(LIB_DIR)help.dl
	install -m 644 texte/messages.dl $(LIB_DIR)messages.dl

log:
	install -d -m 755 $(LOG_DIR)
	install -d -m 755 $(LOG_DIR)fwd/
	install -d -m 755 $(LOG_DIR)http/
	install -d -m 755 $(LOG_DIR)ip/
	install -d -m 755 $(LOG_DIR)sys/
	install -d -m 755 $(LOG_DIR)user/
	install -d -m 755 $(LOG_DIR)spool/

html:
	install -d -m 755 $(HTML_DIR)
	install -m 644 texte/ctext.html.dl $(HTML_DIR)ctext.html.dl
	install -m 644 texte/aktuell.html.dl $(HTML_DIR)aktuell.html.dl
	install -m 644 texte/info.html.dl $(HTML_DIR)info.html.dl
	install -m 644 texte/hinter.gif $(HTML_DIR)hinter.gif
rename: startscript
	mv -f $(BIN_DIR)pager $(BIN_DIR)frmaster
	install -m 755 texte/start_frmaster $(BIN_DIR)start_frmaster
	mv -f $(CONF_DIR)pager.conf $(CONF_DIR)frmaster.conf
	mv -f $(LOG_DIR)pager_fwd $(LOG_DIR)fwd
	mv -f $(LOG_DIR)pager_http $(LOG_DIR)http
	mv -f $(LOG_DIR)pager_ip $(LOG_DIR)ip
	mv -f $(LOG_DIR)pager_sys $(LOG_DIR)sys
	mv -f $(LOG_DIR)pager_user $(LOG_DIR)user
	mv -f $(LOG_DIR)pager_spool $(LOG_DIR)spool

install_conf: install frmasterconf password startscript slaveconf fwdconf log html
	install -d -m 755 $(RUBRIKEN_DIR)
	install -d -m 755 $(FWD_DIR)
	install -d -m 755 $(FWD_DIR)route/
ifeq ($(CREATE_FWD),JA)
	install -d -m 755 $(FWD_DIR)$(FWDCALL)/
endif
	install -d -m 755 $(CONF_DIR)
	install -d -m 755 $(SPOOL_DIR)
ifeq ($(CREATE_SLAVE),JA)
	install -d -m 755 $(SPOOL_DIR)$(SLAVECALL)/
endif
	install -d -m 755 $(GRUPPEN_DIR)
	install -d -m 755 $(LOG_DIR)	
	install -d -m 755 $(IMPORT_DIR)
	install -d -m 755 $(EXPORT_DIR)
	install -m 600 texte/frmaster.conf $(CONF_DIR)frmaster.conf
	install -m 600 texte/passwd $(LIB_DIR)passwd
	install -m 644 texte/crontab $(CONF_DIR)crontab
	install -m 755 texte/start_frmaster $(BIN_DIR)start_frmaster
	install -m 644 texte/info.dl $(LIB_DIR)info.dl
	install -m 644 texte/ctext.dl $(LIB_DIR)ctext.dl
	install -m 644 texte/aktuell.dl $(LIB_DIR)aktuell.dl
	install -m 644 texte/kepler.txt $(LIB_DIR)kepler.txt
	install -m 644 texte/defaults $(LIB_DIR)defaults
ifeq ($(COMPILE_TIDE),JA)
	install -m 644 texte/harmonics $(LIB_DIR)harmonics
endif
	install -m 644 texte/slave.conf $(CONF_DIR)slave.conf
	install -m 644 texte/fwd.conf $(CONF_DIR)fwd.conf
ifeq ($(INSTALL_WX_FILES),JA)
	tar xv -C $(CONF_DIR) -f texte/wx.tgz -z 
endif
	@echo Fertig !!!! $<


frmasterconf:
	@echo Make frmaster.conf $<
	@echo "#"                                                         >$(FRMCONF)
	@echo "#  Konfigurationsdatei fuer das Funkruf-Benutzerinterface">>$(FRMCONF)
	@echo "#"                                                        >>$(FRMCONF)
	@echo "# Format : Parameter = Wert"                              >>$(FRMCONF)
	@echo "#"                                                        >>$(FRMCONF)
	@echo "CTEXT=$(LIB_DIR)ctext"                                    >>$(FRMCONF)
	@echo "#"                                                        >>$(FRMCONF)
	@echo "CTEXT_HTML=$(HTML_DIR)ctext.html"                         >>$(FRMCONF)
	@echo "#"                                                        >>$(FRMCONF)
	@echo "AKTUELL=$(LIB_DIR)aktuell"                                >>$(FRMCONF)
	@echo "#"                                                        >>$(FRMCONF)
	@echo "AKTUELL_HTML=$(HTML_DIR)aktuell.html"                     >>$(FRMCONF)
	@echo "#"                                                        >>$(FRMCONF)
	@echo "INFO=$(LIB_DIR)info"                                      >>$(FRMCONF)
	@echo "#"                                                        >>$(FRMCONF)
	@echo "INFO_HTML=$(HTML_DIR)info.html"                           >>$(FRMCONF)
	@echo "#"                                                        >>$(FRMCONF)
	@echo "HELP=$(LIB_DIR)help"                                      >>$(FRMCONF)
	@echo "#"                                                        >>$(FRMCONF)
	@echo "MESSAGES=$(LIB_DIR)messages"                              >>$(FRMCONF)
	@echo "#"                                                        >>$(FRMCONF)
	@echo "HTMLDIR=$(HTML_DIR)"                                      >>$(FRMCONF)
	@echo "#"                                                        >>$(FRMCONF)
	@echo "HTML_BACKGROUND=/hinter.gif"                              >>$(FRMCONF)
	@echo "#"                                                        >>$(FRMCONF)
	@echo "DATABASE=$(LIB_DIR)database"                              >>$(FRMCONF)
	@echo "#"                                                        >>$(FRMCONF)
	@echo "DEFAULTS=$(LIB_DIR)defaults"                              >>$(FRMCONF)
	@echo "#"                                                        >>$(FRMCONF)
	@echo "KEPLER=$(LIB_DIR)kepler.txt"                              >>$(FRMCONF)
	@echo "#"                                                        >>$(FRMCONF)
	@echo "SAT=LOCK,AO-40,ISS,AO-10"                                 >>$(FRMCONF)
	@echo "#"                                                        >>$(FRMCONF)
	@echo "LOCATOR=$(MYLOC)"                                         >>$(FRMCONF)
	@echo "#"                                                        >>$(FRMCONF)
	@echo "LOGDIR=$(LOG_DIR)"                                        >>$(FRMCONF)
	@echo "#"                                                        >>$(FRMCONF)
	@echo "LOGMASK=3039"                                             >>$(FRMCONF)
	@echo "#"                                                        >>$(FRMCONF)
	@echo "FWDLOGMASK=0"                                             >>$(FRMCONF)
	@echo "#"                                                        >>$(FRMCONF)
	@echo "RPCLOGMASK=0"                                             >>$(FRMCONF)
	@echo "#"                                                        >>$(FRMCONF)
	@echo "SPOOLLOGMASK=0"                                           >>$(FRMCONF)
	@echo "#"                                                        >>$(FRMCONF)
	@echo "CRONTAB=$(CONF_DIR)crontab"                               >>$(FRMCONF)
	@echo "#"                                                        >>$(FRMCONF)
	@echo "PASSWORD=$(LIB_DIR)passwd"                                >>$(FRMCONF)
	@echo "#"                                                        >>$(FRMCONF)
	@echo "PASSWORTFLAGS=0"                                          >>$(FRMCONF)
	@echo "#"                                                        >>$(FRMCONF)
	@echo "SPOOLDIR=$(SPOOL_DIR)"                                    >>$(FRMCONF)
	@echo "#"                                                        >>$(FRMCONF)
	@echo "FORWARDING=NEIN"                                          >>$(FRMCONF)
	@echo "#"                                                        >>$(FRMCONF)
	@echo "FWDDIR=$(FWD_DIR)"                                        >>$(FRMCONF)
	@echo "#"                                                        >>$(FRMCONF)
	@echo "FWDFILE=$(CONF_DIR)fwd.conf"                              >>$(FRMCONF)
	@echo "#"                                                        >>$(FRMCONF)
	@echo "MIDLIST=$(LIB_DIR)messages_ids"                           >>$(FRMCONF)
	@echo "#"                                                        >>$(FRMCONF)
	@echo "DEFAULT_DESTINATION=@DL.EU"                               >>$(FRMCONF)
	@echo "#"                                                        >>$(FRMCONF)
	@echo "GRUPPEN=$(GRUPPEN_DIR)"                                   >>$(FRMCONF)
	@echo "#"                                                        >>$(FRMCONF)
	@echo "RUBRIKEN=$(RUBRIKEN_DIR)"                                 >>$(FRMCONF)
	@echo "#"                                                        >>$(FRMCONF)
	@echo "MYCALL=$(MYCALL)"                                         >>$(FRMCONF)
	@echo "#"                                                        >>$(FRMCONF)
	@echo "INFOBOX_TIMEOUT=30"                                       >>$(FRMCONF)
	@echo "#"                                                        >>$(FRMCONF)
	@echo "AX25_PORT=$(MYPORT)"                                      >>$(FRMCONF)
	@echo "#"                                                        >>$(FRMCONF)
	@echo "AX25_CONNECT=JA"                                          >>$(FRMCONF)
	@echo "#"                                                        >>$(FRMCONF)
	@echo "TCP_CONNECT=JA"                                           >>$(FRMCONF)
	@echo "#"                                                        >>$(FRMCONF)
	@echo "TCP_FWD=JA"                                               >>$(FRMCONF)
	@echo "#"                                                        >>$(FRMCONF)
	@echo "HTTP_SERVER=JA"                                           >>$(FRMCONF)
	@echo "#"                                                        >>$(FRMCONF)
	@echo "AXHTP_SERVER=JA"                                          >>$(FRMCONF)
	@echo "#"                                                        >>$(FRMCONF)
	@echo "AXHTP_PORT=$(MYPORT)"                                     >>$(FRMCONF)
	@echo "#"                                                        >>$(FRMCONF)
	@echo "AXHTP_CALL=$(AXHTPCALL)"                                  >>$(FRMCONF)
	@echo "#"                                                        >>$(FRMCONF)
	@echo "SMTP_SERVER=JA"                                           >>$(FRMCONF)
	@echo "#"                                                        >>$(FRMCONF)
	@echo "SMTP_SERVER_ADRESSE=funkruf.ampr.org"                     >>$(FRMCONF)
	@echo "#"                                                        >>$(FRMCONF)
	@echo "AX_25_SMTP_SENDER="                                       >>$(FRMCONF)
	@echo "#"                                                        >>$(FRMCONF)
ifeq ($(COMPILE_TIDE),JA)
	@echo "#"                                                        >>$(FRMCONF)
	@echo "GEZEITEN=ENABLE,Emden,Wilhelmshaven"                      >>$(FRMCONF)
	@echo "#"                                                        >>$(FRMCONF)
	@echo "HARMONICS=$(LIB_DIR)harmonics"                            >>$(FRMCONF)
	@echo "#"                                                        >>$(FRMCONF)
endif
	@echo "WX=$(CONF_DIR)"                                           >>$(FRMCONF)
	@echo "#"                                                        >>$(FRMCONF)
	@echo "ASTRODATEN=JA"                                            >>$(FRMCONF)
	@echo "#"                                                        >>$(FRMCONF)
	@echo "#          1234567890123456789012345678901234567890123456789012345678901234567890" >>$(FRMCONF)
	@echo "STATUSBAKE=Master $(MYCALL)      %d %t        %u  %U	  "     >>$(FRMCONF)
	@echo "#"                                                        >>$(FRMCONF)
	@echo "IMPORTDIR=$(IMPORT_DIR)"                                  >>$(FRMCONF)
	@echo "#"                                                        >>$(FRMCONF)
	@echo "IMPORT_AS_SYSOP=NEIN"                                     >>$(FRMCONF)
	@echo "#"                                                        >>$(FRMCONF)
	@echo "EXPORTDIR=$(EXPORT_DIR)"                                  >>$(FRMCONF)
	@echo "#"                                                        >>$(FRMCONF)
	@echo "EXPORTMASK=0"                                             >>$(FRMCONF)
	@echo "#"                                                        >>$(FRMCONF)
	@echo "SYSOPS="$(CONFIG_SYSOPS)                                  >>$(FRMCONF)
	@echo "#"                                                        >>$(FRMCONF)
	@echo "SYSOP_INFO_MASK=134"                                      >>$(FRMCONF)
ifeq ($(COMPILE_DIGISTATUS),JA)
	@echo "#"                                                        >>$(FRMCONF)
	@echo "DIGI=$(DIGI_DIR)"					 >>$(FRMCONF)
	@echo "#"                                                        >>$(FRMCONF)
	@echo "DIGI_STATUS=JA"						 >>$(FRMCONF)
endif


password:
	@echo Make password $<
	@echo "00000000011111111112222222222333333333344444444445555555555666666666677777777778">$(PASSWORD)

startscript:
	@echo Make start_frmaster $<
	@echo "#!/bin/bash" 						> $(STARTSCRIPT)
	@echo "#" 							>> $(STARTSCRIPT)
	@echo "export FUNKRUF=$(BASE_DIR)" 				>> $(STARTSCRIPT)
	@echo "#" 							>> $(STARTSCRIPT)
ifeq ($(COMPILE_DEBUG),JA)
	@echo "mv -f $(LOG_DIR)errors.log.2 $(LOG_DIR)errors.log.3"	>> $(STARTSCRIPT)
	@echo "mv -f $(LOG_DIR)errors.log.1 $(LOG_DIR)errors.log.2"	>> $(STARTSCRIPT)
	@echo "mv -f $(LOG_DIR)errors.log $(LOG_DIR)errors.log.1"	>> $(STARTSCRIPT)
	@echo "#"							>> $(STARTSCRIPT)
	@echo "$(BIN_DIR)frmaster 2>$(LOG_DIR)errors.log"			>> $(STARTSCRIPT)
else
	@echo "$(BIN_DIR)frmaster" 						>> $(STARTSCRIPT)
	@echo "#"							>> $(STARTSCRIPT)
	@echo "sleep 60"                                                 >> $(STARTSCRIPT)
endif

slaveconf:
	@echo Make slave.conf $<
	@echo "#" 							> $(SLAVECONF)
	@echo "# Slave-Konfiguration fuer die Funkruf-Master Software" 	>> $(SLAVECONF)
	@echo "#" 							>> $(SLAVECONF)
	@echo "# Keine Slaves vorkonfiguriert"                          >> $(SLAVECONF)


fwdconf:
	@echo Make fwd.conf $<
	@echo "#"                   > $(FWDCONF)
	@echo "# Fwd-Konfiguration fuer die Funkuf-Master-Software " >> $(FWDCONF)
	@echo "#"                                                    >> $(FWDCONF)
	@echo "#" 					            >> $(FWDCONF)
 
#ende