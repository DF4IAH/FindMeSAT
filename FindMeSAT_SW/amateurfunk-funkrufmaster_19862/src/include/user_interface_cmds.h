/****************************************************************************
 *                                                                          *
 *                                                                          *
 * Copyright (C) 2002-2004 by Holger Flemming                               *
 *                                                                          *
 * This Program is free software; you can redistribute ist and/or modify    *
 * it under the terms of the GNU General Public License as published by the *
 * Free Software Foundation; either version 2 of the License, or            *
 * (at your option) any later versions.                                     *
 *                                                                          *
 * This program is distributed in the hope that it will be useful, but      *
 * WITHOUT ANY WARRENTY; without even the implied warranty of               *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General *
 * Public License for more details.                                         *
 *                                                                          *
 * You should have receved a copy of the GNU General Public License along   *
 * with this program; if not, write to the Free Software Foundation, Inc.,  *
 * 675 Mass Ave, Cambridge, MA 02139, USA.                                  *
 *                                                                          *
 ****************************************************************************
 *                                                                          *
 * Author:                                                                  *
 * Holger Flemming, DH4DAI        email : dh4dai@amsat.org                  *
 *                                PR    : dh4dai@db0wts.#nrw.deu.eu         *
 *                                                                          *
 * List of other authors:                                                   *
 * Jens Schoon, DH6BB		  email : dh6bb@darc.de			    *
 *                                PR    : dh6bb@db0whv.#nds.deu.eu          *
 *                                                                          *
 ****************************************************************************/

#ifndef __USER_INTERFACE_CMDS_H__
#define __USER_INTERFACE_CMDS_H__

#include "user_interface.h"


struct user_command
{
  char name[20];
  commands cmd;
};



struct user_command user_commands[200] =
{  
  {  "AKTUELL"       , c_aktuell     },
  {  "ACTIVATE   "   , c_activate    },
  {  "AKTIVIERUNG"   , c_activate    },
  {  "BOARD"         , c_rubrik      },
  {  "CRONTAB"       , c_crontab     },
  {  "CSTATUS"       , c_cstatus     },
  {  "DATE"          , c_zeit        },
  {  "DIR"           , c_dir         },
  {  "DESTINATIONS"  , c_destin      },
  {  "DXCLUSTER"     , c_dxcluster   },
  {  "EXIT"          , c_quit        },
  {  "EXPORTMASK"    , c_exportmask  },
  {  "FUNKRUF"       , c_page        },
  {  "FWDSTATUS"     , c_fwdstatus   },
  {  "GROUP"         , c_groups      },
  {  "GRUPPE"        , c_groups      },
  {  "GPAGE"         , c_gpage       },
  {  "GRUF"          , c_gpage       },
  {  "HELP"          , c_help        },
  {  "HILFE"         , c_help        },
  {  "INFO"          , c_info        },
  {  "INHALT"        , c_dir         },
  {  "LOG"           , c_log         },
  {  "LANGUAGE"      , c_lang        },
  {  "LOCATOR"       , c_locator     },
  {  "NAME"          , c_name        },
  {  "NEWS"          , c_aktuell     },
  {  "PAGE"          , c_page        },
  {  "PASSWD"        , c_passwd      },
  {  "PASSWORT"      , c_passwd      },
  {  "QUIT"          , c_quit        },
  {  "ENDE"          , c_quit        },
  {  "RUBRIK"        , c_rubrik      },
  {  "SATELLITEN"    , c_sat         },
  {  "SHOWBAKEN"     , c_showbaken   },
  {  "SHOWSPOOL"     , c_showspool   },
  {  "SLAVES"        , c_slaves      },
  {  "SPRACHE"       , c_lang        },
  {  "STATISTIK"     , c_statistik   },
  {  "SYSOP"         , c_sysop       },
  {  "TALK"          , c_talk        },
  {  "TYP"           , c_typ         },
  {  "USER"          , c_user        },
  {  "VERSION"       , c_version     },
  {  "WHO"           , c_who         },
  {  "WALL"          , c_wall        },
  {  "ZEIGEBAKEN"    , c_showbaken   },
  {  "ZEIGESPOOL"    , c_showspool   }, // Weiss einer ein deutsches Wort fuer "Spool"verzeichnis?
  {  "ZEIT"          , c_zeit        },
  {  "ZIELE"         , c_destin      },
  {  "?"             , c_help        },
  {  "ADDBAKE"       , c_addbake     },
  {  "ADDBEACON"     , c_addbake     },
  {  "ADDGRP"        , c_addgrp      },
  {  "ADDFWD"        , c_addfwd      },
  {  "ADDSLAVE"      , c_addslave    },
  {  "BEENDE"        , c_shutdown    },
  {  "BEOBACHTE"     , c_trace       },
  {  "CONNECT"       , c_connect     },
  {  "DATEIEN"       , c_filedesc    },
  {  "DBANFORDERN"   , c_dbrequest   },
  {  "DBREQUEST"     , c_dbrequest   },
  {  "DBSAVE"        , c_dbsave      },
  {  "DBSICHERN"     , c_dbsave      },
  {  "DBSENDEN"      , c_dbtrans     },
  {  "DBTRANS"       , c_dbtrans     },
  {  "DELBAKE"       , c_delbake     },
  {  "DELBOARD"      , c_delrub      },
  {  "DELFWD"        , c_delfwd      },
  {  "DELGRP"        , c_delgrp      },
  {  "DELSLAVE"      , c_delslave    },
  {  "DELRUB"        , c_delrub      },
  {  "DIGIPEATER"    , c_digi        },
  {  "ENTFBAKE"      , c_delbake     },
  {  "ENTFFWD"       , c_delfwd      },
  {  "ENTFSENDER"    , c_delslave    },
  {  "ENTFRUB"       , c_delrub      },
  {  "FDESCRIPTOR"   , c_filedesc    },
  {  "FWDLOG"        , c_fwdlog      },
  {  "FWDMASKE"      , c_fwdmask     },
  {  "GEZEITEN"      , c_gezeiten    },
  {  "GRPNAME"       , c_groupname   },
  {  "GRUPPENNAME"   , c_groupname   },
  {  "HTTPLOG"       , c_hlog        },
  {  "IPLOG"         , c_iplog       },
  {  "KILL"          , c_kill        },
  {  "LESETEXT"      , c_rtext       },
  {  "LOGMASKE"      , c_logmask     },
  {  "MKRUB"         , c_mkrub       },
  {  "MKBOARD"       , c_mkrub       },
  {  "MONITOR"       , c_trace       },
  {  "NEUEBAKE"      , c_addbake     },
  {  "NEUEGRUPPE"    , c_addgrp      },
  {  "NEUERFWD"      , c_addfwd      },
  {  "NEUERUBRIK"    , c_mkrub       },
  {  "NEUERSENDER"   , c_addslave    },
  {  "NEWCALL"       , c_newcall     },
  {  "PAGEMASK"      , c_pagemask    },
  {  "PARAMETER"     , c_param       },
  {  "RELAISDB"      , c_relais      },
  {  "RPCLOG"        , c_rpclog      },
  {  "RPCMASKE"      , c_rpcmask     },
  {  "RTEXT"         , c_rtext       },
  {  "RUFMASKE"      , c_pagemask    },
  {  "SAVESLAVES"    , c_saveslaves  },
  {  "SAVEFWD"       , c_savefwd     },
  {  "SCHREIBETEXT"  , c_wtext       },
  {  "SENDE"         , c_send        },
  {  "SETDIR"        , c_setdir      },
  {  "SETFWD"        , c_setfwd      },
  {  "SETSLAVE"      , c_setslave    },
  {  "SETZERUBRIK"   , c_setdir      },
  {  "SETZEFWD"      , c_setfwd      },
  {  "SETZESENDER"   , c_setslave    },
  {  "SICHERESENDER" , c_saveslaves  },
  {  "SICHEREFWD"    , c_savefwd     },
  {  "SHUTDOWN"      , c_shutdown    },
  {  "SLOG"          , c_syslog      },
  {  "SPLOG"         , c_splog       },
  {  "SPLOGMASK"     , c_splogmask   },
  {  "TIDES"         , c_gezeiten    },
  {  "TOETE"         , c_kill        },
  {  "TRACE"         , c_trace       },
  {  "VERBINDE"      , c_connect     },
  {  "WTEXT"         , c_wtext       },
  {  "WX"            , c_wx          },
  {  "+"             , c_adduser     },
  {  "+GRP"          , c_addusrgrp   },
  {  "-"             , c_deluser     },
  {  "-GRP"          , c_delusrgrp   },
  {  ""              , c_no_cmd      }
};

struct user_subcommand
{
  char name[20];
  subcommands cmd;
};



struct user_subcommand user_subcommands[100] =
{
  {  "ACTIV"         , c_sub_aktiv        },
  {  "ADD"           , c_sub_add          },
  {  "ADDBOARDOP"    , c_sub_addboardop   },
  {  "ADDDESTIN"     , c_sub_adddestin    },
  {  "ADDLINK"       , c_sub_addlink      },
  {  "AKTIV"         , c_sub_aktiv        },
  {  "ATV"           , c_sub_atv          },
  {  "BAKEN"         , c_sub_baken        },
  {  "BEACON"        , c_sub_baken        },
  {  "BOEN"          , c_sub_boen         }, // Boen auf Englisch ???
  {  "COMMAND"       , c_sub_command      },
  {  "DEFAULTS"      , c_sub_defaults     },
  {  "DEFLIFETIME"   , c_sub_deflifetime  },
  {  "DEL"           , c_sub_del          },
  {  "DELBOARDOP"    , c_sub_delboardop   },
  {  "DELDESTIN"     , c_sub_deldestin    },
  {  "DELLINK"       , c_sub_dellink      },
  {  "DESTINATIONS"  , c_sub_destinations },
  {  "DIGI"          , c_sub_digi         },
  {  "DISABLE"       , c_sub_disable      },
  {  "DISABLEAR"     , c_sub_ar_disable   },
  {  "ENABLE"        , c_sub_enable       },
  {  "ENABLEAR"      , c_sub_ar_enable    },
  {  "ENTFERNEN"     , c_sub_del          },
  {  "ENTFRUBOP"     , c_sub_delboardop   },
  {  "ENTFZIEL"      , c_sub_deldestin    },
  {  "FEST"          , c_sub_lock         },
  {  "FM10"          , c_sub_fm10         },
  {  "FM2"           , c_sub_fm2          },
  {  "FM70"          , c_sub_fm70         },
  {  "FM23"          , c_sub_fm23         },
  {  "FREIGEBEN"     , c_sub_enable       },
  {  "HARMONICS"     , c_sub_harmonics    },
  {  "HY"            , c_sub_feucht       },  // Was heisst Luftfeuchte auf En.
  {  "KEPLER"        , c_sub_kepler       },
  {  "KOMMANDO"      , c_sub_command      },
  {  "LADE"          , c_sub_load         },
  {  "LEBENSDAUER"   , c_sub_lifetime     },
  {  "LIFETIME"      , c_sub_lifetime     },
  {  "LOAD"          , c_sub_load         },
  {  "LOCATOR"       , c_sub_locator      },
  {  "LOCK"          , c_sub_lock         },
  {  "LUFTDRUCK"     , c_sub_druck        },
  {  "LUFTFEUCHTE"   , c_sub_feucht       },
  {  "MSGREPEAT"     , c_sub_tx_wiederh   },
  {  "MESSAGES"      , c_sub_messages     },
  {  "NACHRICHTEN"   , c_sub_messages     },
  {  "NAMEN"         , c_sub_names        },
  {  "NAMES"         , c_sub_names        },
  {  "NEU"           , c_sub_add          },
  {  "NEUERRUBOP"    , c_sub_addboardop   },
  {  "NEUESZIEL"     , c_sub_adddestin    },
  {  "NIEDERSCHLAG1" , c_sub_nieder1      }, // Niederschlag auf Englisch ???
  {  "NIEDERSCHLAG4" , c_sub_nieder4      },
  {  "NIEDERSCHLAGM" , c_sub_niederm      },
  {  "PASSIV"        , c_sub_passiv       },
  {  "PASSWORD"      , c_sub_passwort     },
  {  "PASSWORT"      , c_sub_passwort     },
  {  "PATH"          , c_sub_pfad         },
  {  "PFAD"          , c_sub_pfad         },
  {  "PRESSURE"      , c_sub_druck        },
  {  "PROMPT"        , c_sub_prompt       },
  {  "RESET"         , c_sub_reset        },
  {  "ROTATE"        , c_sub_rotate       },
  {  "ROTIEREND"     , c_sub_rotate       },
  {  "SHOW"          , c_sub_show         },
  {  "SLOTS"         , c_sub_slots        },
  {  "SPERREN"       , c_sub_disable      },
  {  "SPOOL"         , c_sub_spool        },
  {  "STANDARDS"     , c_sub_defaults     },
  {  "STATUS"        , c_sub_status       },
  {  "STDLEBENSDAUER", c_sub_deflifetime  },
  {  "TEMPERATUR"    , c_sub_temp         },
  {  "TYPE"          , c_sub_typ          },
  {  "WIEDERHOLUNG"  , c_sub_tx_wiederh   },
  {  "WINDDIRECTION" , c_sub_richt        },
  {  "WINDGESCHW"    , c_sub_geschw       },
  {  "WINDRICHTUNG"  , c_sub_richt        },
  {  "WINDGSPEED"    , c_sub_geschw       },
  {  "ZEIGE"         , c_sub_show         },
  {  "ZIELGEBIETE"   , c_sub_destinations },
  {  ""              , c_no_subcmd        }
};
#endif
