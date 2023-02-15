<?php

class frog_suggester {
 public $frog = null;
 public $type = '';
 public $table_name = '';
 public $key_field = 'id';
 public $key_type = 'integer';
 public $value_field = 'title';
 public $fade_field = '';
 public $descriptor_query = '';
 public $search_term;
 public $response;

 function __construct($frog) {
  $this->frog = $frog;
 }

 function run() {
  if (isset($_REQUEST['id'])) {
   $this->send_descriptor();
  } else {
   $this->send_suggestions();
  }
 }

 function send_descriptor() {
  $id = $_REQUEST['id'];

  if ($this->key_type == 'integer') {
   $id = (int) $id;
  } else {
   $id = $this->frog->sanitise($id); 
  }

  if ($this->descriptor_query) {
   $q = sprintf($this->descriptor_query,$id);
   echo $this->get_answer($q);
  } else {
   if ($this->table_name && $this->value_field) {
    if ($this->key_type == 'integer') {
     $key_clause = $this->key_field . '=' . $id; 
    } else {
     $key_clause = $this->key_field . "='$id'";
    }

    $q = "SELECT {$this->value_field} FROM {$this->table_name} WHERE {$key_clause} LIMIT 0,1";
    echo $this->get_answer($q);
   }
  }
 }

 function send_suggestions() {
  if (isset($_REQUEST['s'])) {
   if ($this->frog) {
    $this->search_term = $this->frog->sanitise($_REQUEST['s']);
   } else {
    $this->search_term = preg_replace("/[^A-Za-z0-9,' \-]/", '',$_REQUEST['s']);
   }
  } else {
   $this->search_term = '';
  }

  $this->get_extra_params();

  $this->response = new stdClass();
  $this->response->keys = array();
  $this->response->vals = array();
  $this->response->fade = array();

  $this->get_suggestions();
  
  if (! count ($this->response->fade)) {
   unset($this->response->fade);
  }
  
  header('Content-Type: text/plain');
  echo '(' . json_encode($this->response) . ')';
  exit;
 }

 function get_extra_params() { }

 function get_suggestions() { 
  $s = $this->search_term;

  if ($this->table_name && $this->key_field && $this->value_field) {
   if ($this->fade_field) {
    $q = <<<SQL
SELECT {$this->key_field} AS k,{$this->value_field} AS v,{$this->fade_field} AS f
FROM {$this->table_name}
WHERE {$this->value_field} LIKE '$s%'
ORDER BY f,v
LIMIT 0,10
SQL;
   } else {
    $q = <<<SQL
SELECT {$this->key_field} AS k,{$this->value_field} AS v
FROM {$this->table_name}
WHERE {$this->value_field} LIKE '$s%'
ORDER BY v
LIMIT 0,10
SQL;
   }
   $this->add_suggestions($q);
  }  
 }

 function add_suggestions($query) {
  $results = $this->get_all($query);
  $n = 0;
  foreach($results as $x) {
   if ($this->add_suggestion($x)) { $n++; }
  }
  return($n);
 }

 function add_suggestion($x) {
  if (isset($this->response->vals[$x->k]) &&
      $this->response->vals[$x->k] == $x->v) {
   return false;
  };
  
  $this->response->keys[] = $x->k;
  $this->response->vals[$x->k] = $x->v;
  if (isset($x->f) && $x->f) {
   $this->response->fade[$x->k] = 1;
  }

  return true;
 }

 function get_all($q) {
  $result = $this->frog->db->query($q);
  if ($result === FALSE) {
   $msg = 'Database error in get_all:' . $this->frog->get_error() . '<br/>' .
    "query:\r\n<pre>\r\n$q\r\n</pre>\r\n";
   trigger_error($msg,E_USER_ERROR);
  }

  if ($this->frog->db_type == 'sqlite') {
   $rows = array();
   while($x = $result->fetchArray(SQLITE3_ASSOC)) {
    $rows[] = (object) $x;
   }
  } else {
   $rows= array();
   while($x = mysqli_fetch_object($result)) {
    $rows[] = $x;
   }
  }
  return($rows);
 }

 function get_answer($q) {
  $result = $this->frog->db->query($q);
  if ($result === FALSE) {
    $msg = 'Database error in get_answer:' . $this->frog->get_error() . '<br/>' .
     "query:\r\n<pre>\r\n$q\r\n</pre>\r\n";
   trigger_error($msg,E_USER_ERROR);
  }

  $x = $result->fetch_row();
  if ($x === FALSE) {
   return('');
  } else {
   return($x[0]);
  }
 }
}


?>
