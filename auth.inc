<?php

class frog_auth {
 var $frog = null;
 var $logged_in_key;
 var $token_key;
 var $user_id_key;
 var $is_logged_in;
 var $user_table = 'user';
 var $user_key = 'user_name';
 var $user_key_regex = '/^[A-Za-z0-9]+$/i';
 var $user_is_current_clause = "x.is_expired=0";
 var $use_ldap = false;
 var $ldap_server;
 var $ldap_base_dn;
 var $login_log_file;
 
 function __construct($frog) {
  $this->frog = $frog;
  $keys = array();

  if (isset($frog->cfg)) {
   foreach ($keys as $k) {
    if (isset($frog->cfg->$k)) {
     $this->$k = $frog->cfg->$k;
    }
   }
  }

  $this->logged_in_key = $frog->name . '_logged_in';
  $this->token_key     = $frog->name . '_token';
  $this->user_id_key   = $frog->name . '_user_id';
 }

 function require_login() {
  global $user;
  
  if (isset($_SESSION[$this->logged_in_key]) &&
      $_SESSION[$this->logged_in_key]) {
   $user_id = $_SESSION[$this->user_id_key];
   $this->frog->load_table($this->user_table);
   $user = $this->frog->load($this->user_table,$user_id);
   if ($user) {
    $this->is_logged_in = true;
   } else {
    $this->is_logged_in = false;
    $this->frog->nav->request_login_page();
    exit;
   }
  } else {
   $this->is_logged_in = false;
   $this->frog->nav->request_login_page();
   exit;
  }
 }

 function require_token() {
  if (! (isset($_REQUEST[$this->token_key]) &&
	 $_REQUEST[$this->token_key] == $_SESSION[$this->token_key])) {
   exit;
  }
 }

 function handle_login($params) {
  $d = date('Y-m-d H:i:s');
  $msg = <<<TEXT

Login attempt
Time: $d
Remote Host: {$_SERVER['REMOTE_ADDR']}
User Agent:  {$_SERVER['HTTP_USER_AGENT']}
TEXT;

  $this->write_login_log($msg);

  if ($i = session_id()) {
   $this->write_login_log("Session already started: $i");
  } else {
   session_start(); 
   $i = session_id();
   $this->write_login_log("New session: $i");
  }

  $f = $this->frog;
  $k = $this->user_key;
  if ($params->$k) {
   $w = $k . "='" . $f->sanitise($params->$k) . "' AND " .
         $this->user_is_current_clause;
   $f->load_table($this->user_table);
   $uu = $f->load_where($this->user_table,$w);
   if (count($uu) != 1) {
    if (! $uu) {
     $params->login_error = 'user_key not found';
    } else {
     $params->login_error = 'multiple matches for user_key';
    }
   
    $this->frog->nav->request_login_page($params);
    exit;
   }

   $params->user = $uu[0];
   $x = $this->check_password($params);
   if ($x->ok) {
    $_SESSION[$this->logged_in_key] = true;
    $_SESSION[$this->token_key] = bin2hex(openssl_random_pseudo_bytes(16));
    $_SESSION[$this->user_id_key] = $params->user->id;
    $this->after_login($params);
    $this->redirect_to_requested_url($params);
   } else {
    $this->write_login_log('Password error: ' . $x->error);
    $params->login_error = $x->error;
    $this->frog->nav->request_login_page($params);
   }
  } else {
   $this->frog->nav->request_login_page();
  }
 }

 function check_password($params) {
  return $this->hash_check_password($params->user->password_hash,
				    $params->password);
 }
 
 function hash_check_password($hash,$password) {
  $x = new stdClass();
  $x->ok = false;
  $x->error = '';
  
  if (! $password) {
   $x->ok = false;
   $x->error = 'password empty';
   return $x;
  }

  if (! $hash) {
   $x->ok = false;
   $x->error = 'hash empty';
   return $x;
  }

  if (! password_verify($password,$hash)) {
   $x->ok = false;
   $x->error = 'password incorrect (hash)';
   return $x;
  }

  $x->ok = true;
  return $x;
 }
 
 function ldap_check_password($user_key,$password) {
  $x = new stdClass();
  $x->ok = false;
  $x->error = '';
  
  if (! $user_key) {
   $x->ok = false;
   $x->error = 'user_key empty';
   return $x;
  }

  if (! $password) {
   $x->ok = false;
   $x->error = 'password empty';
   return $x;
  }

  if ( ! preg_match($this->user_key_regex,$user_key)) {
   $x->ok = false;
   $x->error = 'malformed user_key';
   return $x;
  }

  $ds = ldap_connect($this->ldap_server);
  $r  = ldap_bind( $ds );
  $sr = ldap_search( $ds, $this->ldap_base_dn, "uid=$user_key" );
  
  if ( ldap_count_entries( $ds, $sr ) != 1 ) {
   $x->ok = false;
   $x->error = "user_key ($user_key) not found";
   return $x;
  }

  $ldap_entry = ldap_get_entries( $ds, $sr );
  $user_dn = $ldap_entry[0]['dn'];

  // now, to verify password, try to bind as this user with the supplied password
  $authenticated = @ldap_bind( $ds, $user_dn, $password );

  if ($authenticated) {
   $x->ok = true;
   return $x;
  } else {
   $x->ok = false;
   $x->error = 'password incorrect (ldap)';
   return $x;
  }
 }

 function redirect_to_requested_url($params) {
  $url = '';
  if (isset($_REQUEST['requested_url'])) {
   $url = $_REQUEST['requested_url'];
  }

  if ($url) {
   $parts = parse_url($url);
   if (basename($parts['path']) == 'login.php') {
    $url = $this->frog->nav->index_url;
   }
  } else {
   $url = $this->frog->nav->index_url;
  }

  header('Location: ' . $url);
 }

 function write_login_log($msg) {
  if ($this->login_log_file) {
   file_put_contents($this->login_log_file,
		     $msg . PHP_EOL,
		     FILE_APPEND);
  }
 }

 function handle_logout() {
  session_destroy();
  $this->after_logout();
  header('Location: ' . $this->frog->nav->base_url . '/login.php');
 }

 function after_login($params) {}
 function after_logout() {} 
}

?>
