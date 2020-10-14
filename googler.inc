<?php

class frog_googler {
 var $log_file = '';
 var $credentials_dir = '';
 var $clients = array();
 var $auth_uris = array();
 var $access_tokens = array();
 var $refresh_tokens = array();
 var $account_email = '';
 var $account_name = '';
 
 function write_log($msg) {
  if ($this->log_file) {
   file_put_contents($this->log_file,$msg . PHP_EOL,FILE_APPEND);
  }
 }

 function get_account($force = false) {
  if ($this->account_email && ! $force) { return null; }
  $x = json_decode(file_get_contents($this->credentials_dir . '/account.json'));
  $this->account_name = $x->account_name;
  $this->account_email = $x->account_email;
 }
 
 function get_client($key,$create = false) {  // $key will be 'gmail' or 'gcal'
  global $user;

  if (isset($this->clients[$key]) && ! $create) {
   return $this->clients[$key];
  }
  
  $client = new Google_Client();
  $this->clients[$key] = $client;
  $config0 = json_decode(file_get_contents($this->credentials_dir . "/{$key}.json"));
  $config = $config0->web;
  $client->setClientId($config->client_id);
  $client->setClientSecret($config->client_secret);
  $client->setAccessType('offline');

  if (! isset($this->access_tokens[$key])) {
   $this->access_tokens[$key] = null;
  }
  
  if (! isset($this->refresh_tokens[$key])) {
   $this->refresh_tokens[$key] = null;
  }

  if (isset($user) && $user) {
   $msg = "------------------------------------------------\n";
   $msg .= date('Y-m-d H:i:s') . ' Creating client for ' . $user->full_name . "\n";
   $msg .= "\n";
   $this->write_log($msg);
  } else {
   $msg = "------------------------------------------------\n";
   $msg .= date('Y-m-d H:i:s') . " Creating client\n";
   $msg .= "\n";
   $this->write_log($msg);
  }

  return $client;
 }

 function get_access_token($key) {
  $key_client        = $key . '_client';
  $key_access_token  = $key . '_access_token';
  $key_refresh_token = $key . '_refresh_token';

  $access_token_file  = $this->credentials_dir . '/' . $key . '_access_token.json';
  $refresh_token_file = $this->credentials_dir . '/' . $key . '_refresh_token.txt';

  if (file_exists($access_token_file)) {
   $access_token_json = file_get_contents($access_token_file);
   $access_token_arr  = json_decode($access_token_json,true);
  } else {
   $access_token_json = '';
   $access_token_arr = array();
  }

  if (file_exists($refresh_token_file)) {
   $refresh_token = file_get_contents($refresh_token_file);
  } else {
   $refresh_token = '';
  }
  
  $this->access_tokens[$key] = $access_token_arr;
  $this->refresh_tokens[$key] = $refresh_token;
                   
  $msg = "------------------------------------------------\n";
  $msg .= date('Y-m-d H:i:s') . ": In get_access_token, key=$key\n";
  $msg .= "access_token = " . $access_token_json . "\n";
  $msg .= "refresh_token = " . $refresh_token . "\n\n";

  $this->write_log($msg);

  $access_token_ok = $access_token_arr ? true : false;

  $client = $this->get_client('gmail');
  
  if ($access_token_ok) {
   try {
    $client->setAccessToken($access_token_arr);
   } catch (Exception $x) {
    $msg = "------------------------------------------------\n";
    $msg .= date('Y-m-d H:i:s') . ": Error in setAccessToken()\n";
    $msg .= var_export($access_token_arr,true);
    $msg .= var_export($x,true);
    $msg .= "\n";
    $this->write_log($msg);
    $access_token_ok = false;
   }
  }
  
  if (! $access_token_ok || $client->isAccessTokenExpired()) {
   $msg .= "access_taken is null or expired\n";
  
   try {
    $client->refreshToken($refresh_token);
   } catch (Exception $x) {
    $msg = "------------------------------------------------\n";
    $msg .= date('Y-m-d H:i:s') . ": Error in refreshToken()\n";
    $msg .= var_export($refresh_token,true);
    $msg .= var_export($x,true);
    $msg .= "\n";
    $this->write_log($msg);
    $access_token_ok = false;
   }
  }

  try {
   $access_token_arr = $client->getAccessToken();
  } catch (Exception $x) {
   $msg = "------------------------------------------------\n";
   $msg .= date('Y-m-d H:i:s') . ": Error in getAccessToken()\n";
   $msg .= var_export($x,true);
   $msg .= "\n";
   $this->write_log($msg);
   return false;
  }
  
  $access_token_json = json_encode($access_token_arr);
  $msg = "------------------------------------------------\n";
  $msg .= date('Y-m-d H:i:s') . ": New token\n";
  $msg .= "New access_token = " . $access_token_json . "\n";
  
  if ($access_token_json) {
   file_put_contents($access_token_file,$access_token_json);
   $msg .= "New access_token saved \n\n";
   $this->write_log($msg);
   return true;
  } else {
   $msg .= "New access_token was empty and so was not saved \n";
   $this->write_log($msg);
   return false;
  }
 }

 function get_scope($key) {
  if ($key == 'gcal') {
   return "https://www.googleapis.com/auth/calendar";
  } else if ($key == 'gmail') {
   return array('https://mail.google.com/',
         'https://www.googleapis.com/auth/gmail.compose',
         'https://www.googleapis.com/auth/gmail.send');
  } else {
   trigger_error('Unknown key');
   exit;
  }
 }

 function redirect_to_authorise($key) {
  $client = $this->get_client($key);
  $client->setScopes($this->get_scope($key));
  $client->setRedirectUri($this->auth_uris[$key]);
  $client->setAccessType('offline');
  $client->setApprovalPrompt('force');
  $auth_url = $client->createAuthUrl();
  header('Location: ' . filter_var($auth_url, FILTER_SANITIZE_URL));
 }

 function handle_auth_code($key,$code,$nav = null) {
  $key_client        = $key . '_client';
  $key_access_token  = $key . '_access_token';
  $key_refresh_token = $key . '_refresh_token';
  
  $access_token_file  = $this->credentials_dir . '/' . $key . '_access_token.json';
  $refresh_token_file = $this->credentials_dir . '/' . $key . '_refresh_token.txt';

  $client = $this->get_client($key);
  $client->setScopes($this->get_scope($key));
  $client->setRedirectUri($this->auth_uris[$key]);
  $client->authenticate($code);
  
  $access_token_arr = $client->getAccessToken();
  $access_token_json = json_encode($access_token_arr);
 
  if (isset($access_token_arr['access_token']) &&
      isset($access_token_arr['refresh_token'])) {
   $access_token  = $access_token_arr['access_token'];
   $refresh_token = $access_token_arr['refresh_token'];
   file_put_contents($access_token_file,$access_token_json);
   file_put_contents($refresh_token_file,$refresh_token);

   if ($nav) { $nav->header("$key setup"); }

   echo <<<HTML
<h1>Google $key Token Setup</h1>

<div class="text">
New tokens
<table class="edged">
 <tr>
  <td width="100">Refresh token:</td>
  <td width="600">$refresh_token</td>
 </tr>
 <tr>
  <td width="100">Access token:</td>
  <td width="600">$access_token</td>
 </tr>
</table>

HTML
    ;

   if ($nav) { $nav->footer(); }
  } else {
   $msg = 
        'Unexpected return value from getAccessToken: <br/>' .
        $token_json;
  
   if ($nav) {
    $nav->error_page($msg);
   } else {
    echo $msg;
   }
  }
 }
 
 function handle_auth_error($key,$error,$nav = null) {
  if ($nav) { $nav->header("$key Setup"); }

  echo <<<HTML
  The Google OAuth page reported an error as follows:
<br/><br/>
$error
<br/><br/>

HTML
   ;

  if ($nav) { $nav->footer(); }
 }

 function get_mailer() {  
  $this->get_account();
  $this->get_access_token('gmail');
  $client = $this->get_client('gmail');

  $mail = new \PHPMailer\PHPMailer\PHPMailer();
  $mail->isSMTP();
  $mail->Host       = "smtp.gmail.com";
  $mail->Port       = 587;
  $mail->SMTPSecure = \PHPMailer\PHPMailer\PHPMailer::ENCRYPTION_STARTTLS;
  $mail->SMTPAuth   = true;
  $mail->AuthType   = 'XOAUTH2';

  $client_id = $client->getClientId();
  $client_secret = $client->getClientSecret();
  
  $provider = new \League\OAuth2\Client\Provider\Google([
   'clientId' => $client_id,
   'clientSecret' => $client_secret
  ]);

  $mail->setOAuth(
   new \PHPMailer\PHPMailer\OAuth(
    [
     'provider' => $provider,
     'clientId' => $client_id,
     'clientSecret' => $client_secret,
     'refreshToken' => $this->refresh_tokens['gmail'],
     'userName' => $this->account_email
    ]
   )
  );
  
  $mail->From              = $this->account_email;
  $mail->FromName          = $this->account_name; 

  return $mail;
 }
}

