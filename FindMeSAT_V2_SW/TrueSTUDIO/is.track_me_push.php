<?php

$outH = fopen('file:///tmp/php.out', 'w');

/* if started from commandline, wrap parameters to $_POST and $_GET */
if (!isset($_SERVER["HTTP_HOST"])) {
  $postAry = array();

  //parse_str($argv[1], $_GET);
  //parse_str($argv[1], $_POST);

  $usrPwd  = "track_me:So§Let5MeIn";
  $auth    = "Authorization: Basic ";
  $auth   .= base64_encode($usrPwd);
  $auth   .= "\r\n";

  $inH = fopen('php://stdin','r');
  while (!feof($inH)) {
    $postAry[] = fgets($inH);
  }
  fclose($inH);

  $stat = print_r($postAry, TRUE);
  fputs($outH, $stat);

  $json = $postAry[9];
  $appAry = json_decode($json, TRUE);
  $stat = print_r($appAry, TRUE);
  fputs($outH, $stat);

  if (!strcmp($auth, $postAry[4])) {
    fputs($outH, "User/PWD OK\r\n");

    //string openssl_encrypt ( string $data , string $method , string $key [, int $options = 0 [, string $iv = "" [, string &$tag = NULL [, string $aad = "" [, int $tag_length = 16 ]]]]] );
    $tsKeyCmp = ((0x0392ea84 + ($appAry['payload_fields']['ts'] & 0x7fffffff)) % 0x17ef23ab) ^ ((0x339dc627 + ($appAry['payload_fields']['ts'] & 0x07ffffff) * 7) % 0x2a29e2cc);

    //fprintf($outH, "tsKeyCM  = %x\r\n", $appAry['payload_fields']['cm']);
    //fprintf($outH, "tsKeyCmp = %x\r\n", $tsKeyCmp);

    if ($appAry['payload_fields']['cm'] == $tsKeyCmp && $appAry['payload_fields']['latitude'] && $appAry['payload_fields']['longitude']) {
      $aprs_source_callsign = "";
      
      /* Database access */
      {
        fputs($outH, "UNLOCKED - now processing data\r\n");
  
        // Database access
        $dbconn = pg_connect("host=localhost dbname=track_me user=track_me_push password=Push1And.Go");
  
        // First: handle devs table
        $query = "SELECT id, dev_id, ham_call  FROM devs  WHERE dev_eui = '" . $appAry['hardware_serial'] . "'";
        $result = pg_query($query);
        if (!$result) {
          fprintf($outH, "SQL-Error: %s\r\n", pg_last_error());
          fprintf($outH, "Query was: %s\r\n", $query);
          die('PQSQL query error');
        }
  
        $rc = pg_num_rows($result);
        if ($rc == 0) {
          pg_free_result($result);
  
          // Insert new device
          $query = "INSERT INTO devs(dev_id, dev_eui)  VALUES('" . $appAry['dev_id'] . "', '" . $appAry['hardware_serial'] . "');";
          $result = pg_query($query);
          //fprintf($outH, "SQL-Error: %s\r\n", pg_last_error());
          //fprintf($outH, "Query was: %s\r\n", $query);
  
        } else {
          // Get Device IDs
          $line                 = pg_fetch_array($result, null, PGSQL_ASSOC);
          $db_dev__id           = $line['id'];
          $db_dev__dev_id       = $line['dev_id'];
          $aprs_source_callsign = $line['ham_call'];
  
          fprintf($outH, "compare old dev_id=%s with new dev_id=%s\r\n", $db_dev__dev_id, $appAry['dev_id']);
  
          if (strcmp($db_dev__dev_id, $appAry['dev_id'])) {
            pg_free_result($result);
  
            // Update dev_id
            $query = "UPDATE devs  SET ts_last = now(), dev_id = '" . $appAry['dev_id'] . "'  WHERE id = " . $db_dev__id . ";";
            $result = pg_query($query);
            //fprintf($outH, "SQL-Error: %s\r\n", pg_last_error());
            //fprintf($outH, "Query was: %s\r\n", $query);
          }
        }
        pg_free_result($result);
  
        // Second: handle trks table
        $db_trks__lat       = $appAry['payload_fields']['latitude'];
        $db_trks__lon       = $appAry['payload_fields']['longitude'];
        $db_trks__alt       = $appAry['payload_fields']['altitude'];
        $db_trks__accuracy  = $appAry['payload_fields']['accuracy'];
        $db_trks__spd       = $appAry['payload_fields']['speed'];
        $db_trks__crs       = $appAry['payload_fields']['course'];
        $db_trks__vspd      = $appAry['payload_fields']['vertspeed'];
        $db_trks__temp      = $appAry['payload_fields']['temp'];
        $db_trks__rh        = $appAry['payload_fields']['hygro'];
        $db_trks__baro      = $appAry['payload_fields']['baro'];
        $db_trks__dust025   = $appAry['payload_fields']['dust025'];
        $db_trks__dust100   = $appAry['payload_fields']['dust100'];
        $db_trks__sievert   = $appAry['payload_fields']['sievert'];
        $db_trks__vbat      = $appAry['payload_fields']['vbat'];
        $db_trks__ibat      = $appAry['payload_fields']['ibat'];
  
        $query  = "INSERT INTO trks(devs_fk, lat, lon, accuracy, alt, spd, crs, vspd, temp, rh, baro, dust025, dust100, sievert, vbat, ibat)  ";
        $query .= "VALUES(" . $db_dev__id . ", " . $db_trks__lat . ", " . $db_trks__lon . ", " . $db_trks__accuracy;
  
        if ($db_trks__alt) {
          $query  .= ", " . $db_trks__alt;
        } else {
          $query  .= ", NULL";
        }
  
        if ($db_trks__spd) {
          $query  .= ", " . $db_trks__spd;
        } else {
          $query  .= ", NULL";
        }
  
        if ($db_trks__crs) {
          $query  .= ", " . $db_trks__crs;
        } else {
          $query  .= ", NULL";
        }
  
        if ($db_trks__vspd) {
          $query  .= ", " . $db_trks__vspd;
        } else {
          $query  .= ", NULL";
        }
  
        if ($db_trks__temp) {
          $query  .= ", " . $db_trks__temp;
        } else {
          $query  .= ", NULL";
        }
  
        if ($db_trks__rh) {
          $query  .= ", " . $db_trks__rh;
        } else {
          $query  .= ", NULL";
        }
  
        if ($db_trks__baro) {
          $query  .= ", " . $db_trks__baro;
        } else {
          $query  .= ", NULL";
        }
  
        if ($db_trks__dust025) {
          $query  .= ", " . $db_trks__dust025;
        } else {
          $query  .= ", NULL";
        }
  
        if ($db_trks__dust100) {
          $query  .= ", " . $db_trks__dust100;
        } else {
          $query  .= ", NULL";
        }
  
        if ($db_trks__sievert) {
          $query  .= ", " . $db_trks__sievert;
        } else {
          $query  .= ", NULL";
        }
  
        if ($db_trks__vbat) {
          $query  .= ", " . $db_trks__vbat;
        } else {
          $query  .= ", NULL";
        }
  
        if ($db_trks__ibat) {
          $query  .= ", " . $db_trks__ibat;       
        } else {
          $query  .= ", NULL";
        }
  
        $query .= ");";
  
        $result = pg_query($query);
        fprintf($outH, "SQL-Error: %s\r\n", pg_last_error());
        fprintf($outH, "Query was: %s\r\n", $query);
  
        pg_free_result($result);
        pg_close($dbconn);
      }
    }
    
    /* LoRa track_me application to APRS gateway */
    {
      $PM_APRS_TX_HTTP_L1         = "POST / HTTP/1.1\r\n";
      $PM_APRS_TX_HTTP_L2         = "Content-Length: %d\r\n";
      $PM_APRS_TX_HTTP_L3         = "Content-Type: application/octet-stream\r\n";
      $PM_APRS_TX_HTTP_L4         = "Accept-Type: text/plain\r\n\r\n";
      $PM_APRS_TX_LOGIN           = "user %s pass %s vers %s %s\r\n";
      $PM_APRS_TX_FORWARD         = "%s>APRS,TCPIP*:";                              // USER, SSID with prefixing "-"
      $PM_APRS_TX_SYMBOL_TABLE_ID = "/";
      $PM_APRS_TX_SYMBOL_CODE     = "j";  // /j: Jeep, /R: RV, \k: SUV, /W: WX
      $PM_APRS_TX_POS_SPD_HDG     = "!%02d%5.2f%s%s%03d%5.2f%s%s%03d/%03d";
      $PM_APRS_TX_N1              = "%cGx=%+06.1fd Gy=%+06.1fd Gz=%+06.1fd";
      $PM_APRS_TX_N2              = "%cAx=%+6.3fg Ay=%+6.3fg Az=%+6.3fg";
      $PM_APRS_TX_N3              = "%cMx=%+6.1fuT My=%+6.1fuT Mz=%+6.1fuT";
      $PM_APRS_TX_N4              = "%c/A=%06ld DP=%+5.2fC QNH=%7.2fhPa";
      $PM_APRS_TX_MSGEND          = "\r\n";
    
      /* Prepare data */
      $app__lat = $appAry['payload_fields']['latitude'];
      $app__lon = $appAry['payload_fields']['longitude'];
      $app__alt = $appAry['payload_fields']['altitude'];
      $app__spd = $appAry['payload_fields']['speed'];
      $app__crs = $appAry['payload_fields']['course'];
      
      if ($app__lat >= 0) {
        $l_lat_deg_d = floor( $app__lat);
        $l_lat_minutes_f = 60 * ($app__lat - $l_lat_deg_d);
        $l_lat_hemisphere = "N";

      } else {
        $l_lat_deg_d = floor(-$app__lat);
        $l_lat_minutes_f = 60 * (-$app__lat - $l_lat_deg_d);
        $l_lat_hemisphere = "S";
      }
      
      if ($app__lon >= 0) {
        $l_lon_deg_d = floor( $app__lon);
        $l_lon_minutes_f = 60 * ($app__lon - $l_lon_deg_d);
        $l_lon_hemisphere = "E";

      } else {
        $l_lon_deg_d = floor(-$app__lon);
        $l_lon_minutes_f = 60 * (-$app__lon - $l_lon_deg_d);
        $l_lon_hemisphere = "W";
      }

      $l_course_deg = $app__crs;
      $l_speed_kn   = $app__spd * 3.6 / 1.852;  // m/s --> kn
      
      /* APRS constants */
      $g_aprs_login_user          = "DF4IAH";
      $g_aprs_login_pwd           = "17061";
      $APPLICATION_NAME           = "LoRa-TrackMe_APRS-GW";
      $APPLICATION_VERSION        = "1.0";
      
      /* Message content APRS */
      $aprs_content_msg  = sprintf($PM_APRS_TX_FORWARD, $aprs_source_callsign);
      $aprs_content_msg .= sprintf($PM_APRS_TX_POS_SPD_HDG, $l_lat_deg_d, $l_lat_minutes_f, $l_lat_hemisphere, $PM_APRS_TX_SYMBOL_TABLE_ID, $l_lon_deg_d, $l_lon_minutes_f, $l_lon_hemisphere, $PM_APRS_TX_SYMBOL_CODE, $l_course_deg, $l_speed_kn);
    //$aprs_content_msg .= sprintf($PM_APRS_TX_N1, $l_mark, $l_aprs_alert_1_gyro_x_mdps / 1000.f, $l_aprs_alert_1_gyro_y_mdps / 1000.f, $l_aprs_alert_1_gyro_z_mdps / 1000.f);
    //$aprs_content_msg .= sprintf($PM_APRS_TX_N2, $l_mark, $l_aprs_alert_1_accel_x_mg / 1000.f, $l_aprs_alert_1_accel_y_mg / 1000.f, $l_aprs_alert_1_accel_z_mg / 1000.f);
    //$aprs_content_msg .= sprintf($PM_APRS_TX_N3, $l_mark, $l_aprs_alert_2_mag_x_nT / 1000.f, $l_aprs_alert_2_mag_y_nT / 1000.f, $l_aprs_alert_2_mag_z_nT / 1000.f);
    //$aprs_content_msg .= sprintf($PM_APRS_TX_N4, $l_mark, $l_gns_msl_alt_ft, $l_twi1_hygro_DP_100 / 100.f, $l_twi1_baro_p_h_100 / 100.f);
      $aprs_content_msg .= $PM_APRS_TX_MSGEND;
      
      $aprs_content_hdr = sprintf($PM_APRS_TX_LOGIN, $g_aprs_login_user, $g_aprs_login_pwd, $APPLICATION_NAME, $APPLICATION_VERSION);
      $len = strlen($aprs_content_hdr) + 2 + strlen($aprs_content_msg);

      /* Line 1 */
      $aprs_msg  = sprintf($PM_APRS_TX_HTTP_L1);

      /* Line 2 */
      $aprs_msg .= sprintf($PM_APRS_TX_HTTP_L2, $len);

      /* Line 3 */
      $aprs_msg .= sprintf($PM_APRS_TX_HTTP_L3);

      /* Line 4 */
      $aprs_msg .= sprintf($PM_APRS_TX_HTTP_L4);

      /* Content header - authentication */
      $aprs_msg .= $aprs_content_hdr;

      /* Content message */
      $aprs_msg .= $aprs_content_msg;

      /* Ending line */
      $aprs_msg .= $PM_APRS_TX_MSGEND;

      /* Print APRS message for debugging */
      $dbgH = fopen('file:///tmp/php_dbg.out', 'w');
      fputs($dbgH, $aprs_msg);
      fclose($dbgH);

      /* Open destination port */
      $sockH = stream_socket_client("tcp://nuremberg.aprs2.net:8080", $errno, $errstr, 5);
      fputs($sockH, $aprs_msg);
      fclose($sockH);
    }
  }
}
fclose($outH);

?>