<?php

class frog_mailer {
 var $fake_recipient = null;
 
 function mail($to,$subject,$body,$from) {
  if ($this->fake_recipient) {
   return $this->mail_aux($this->fake_recipient,"$subject [$to]",$from);
  } else {
   return $this->mail_aux($to,$subject,$body,$from);
  }
 }

 function mail_aux($to,$subject,$body,$from) {
  mail($to,$subject,$body,$from);
 }
}

class frog_gmailer extends frog_mailer {
 var $googler = null;
 
 function __construct($d) {
  $this->googler = new frog_googler();
  $this->googler->credentials_dir = $d;
 }
 
 function mail_aux($to,$subject,$body,$from) {
  $m = $this->googler->get_mailer();
  $m->addAddress($to);
  $m->Subject = $subject;
  $m->Body = $body;
  return $m->Send();
 }
}
