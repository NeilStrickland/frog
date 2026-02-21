<?php

class frog {
 var $name;
 var $db;
 var $registry;
 var $subtype_registry;
 var $include_dir;
 var $data_dir;
 var $config;
 var $cache;
 var $db_name = '';
 var $db_type = 'mysql';
 var $db_host = 'localhost';
 var $db_port = 3306;
 var $db_file = '';

 function __construct() {
  $this->registry = array();
  $this->config = array();
  $this->cache = array();
 }

 function connect($db_user = '',$db_pass = '') {
  if ($this->db_type == 'sqlite') {
    $this->connect_sqlite();
  } else {
    $this->connect_mysql($db_user,$db_pass);
  }
 }

 function connect_sqlite() {
  $this->db = new SQLite3($this->db_file);
 }

 function connect_mysql($db_user,$db_pass) {
  $n = $this->db_name ? $this->db_name : $db_user;
  $this->db = mysqli_connect($this->db_host,$db_user,$db_pass,$n,$this->db_port);

  if (!$this->db) {
   $msg = 'Could not connect to database: ' . mysqli_connect_error();
   trigger_error($msg,E_USER_ERROR);
   exit;
  }

  $this->db->set_charset('utf8');
 }
 
 function escape_string($s) {
  if ($this->db_type == 'sqlite') {
    return SQLite3::escapeString($s);
  } else {
    return $this->db->real_escape_string($s);
  }
 } 

 function export_schema_text() {
  if ($this->db_type != 'sqlite') {
   $schema_text = 'Schema export is only implemented for sqlite.' . PHP_EOL;
   $schema_file = $this->data_dir . '/schema.txt';
   file_put_contents($schema_file, $schema_text);
   return $schema_file;
  }

  $lines = array();
  $tables = $this->get_all("SELECT name FROM sqlite_master WHERE type='table' AND name NOT LIKE 'sqlite_%' ORDER BY name");
  foreach ($tables as $t) {
   $table_name = $t->name;
   $lines[] = "TABLE {$table_name}";

   $cols = $this->get_all("PRAGMA table_info('" . $this->escape_string($table_name) . "')");
   foreach ($cols as $c) {
    $flags = array();
    if ($c->pk) { $flags[] = 'PK'; }
    if ($c->notnull) { $flags[] = 'NOT NULL'; }
    if ($c->dflt_value !== null && $c->dflt_value !== '') {
     $flags[] = 'DEFAULT ' . $c->dflt_value;
    }
    $flag_text = $flags ? ' [' . implode(', ', $flags) . ']' : '';
    $lines[] = "  - {$c->name} {$c->type}{$flag_text}";
   }

   $fks = $this->get_all("PRAGMA foreign_key_list('" . $this->escape_string($table_name) . "')");
   if ($fks && count($fks) > 0) {
    $lines[] = '  FOREIGN KEYS';
    foreach ($fks as $fk) {
     $lines[] = "    - {$fk->from} -> {$fk->table}.{$fk->to} (on update {$fk->on_update}, on delete {$fk->on_delete})";
    }
   }

   $indexes = $this->get_all("PRAGMA index_list('" . $this->escape_string($table_name) . "')");
   if ($indexes && count($indexes) > 0) {
    $lines[] = '  INDEXES';
    foreach ($indexes as $ix) {
     $index_name = $ix->name;
     $index_cols = $this->get_all("PRAGMA index_info('" . $this->escape_string($index_name) . "')");
     $col_names = array();
     foreach ($index_cols as $ic) {
      $col_names[] = $ic->name;
     }
     $unique = $ix->unique ? ' UNIQUE' : '';
     $lines[] = "    - {$index_name}{$unique} (" . implode(', ', $col_names) . ')';
    }
   }

   $lines[] = '';
  }

  if (count($lines) == 0) {
   $lines[] = 'No tables found';
  }

  $schema_text = rtrim(implode(PHP_EOL, $lines)) . PHP_EOL;
  $schema_file = $this->data_dir . '/schema.txt';
  file_put_contents($schema_file, $schema_text);
  return $schema_file;
 }

 function load_config() {
  $xx = $this->get_all('SELECT * FROM tbl_config');

  if (! isset($this->config)) { $this->config = array(); }
  foreach($xx as $x) {
   $this->config[$x->key] = $x->value;
  }
 }

 function get_config($key) {
  $k = $this->escape_string($key);
  $xx = $this->get_all("SELECT * FROM tbl_config WHERE `key`='$k'");
  if ($xx) {
   if (! isset($this->config)) { $this->config = array(); }
   $this->config[$xx[0]->key] = $xx[0]->value;
   return($xx[0]->value);
  } else {
   return(null);
  }
 }

 function set_config($key,$value) {
  global $db_somas;
  $k = $this->escape_string($key);
  $v = $this->escape_string($value);
  $xx = $this->get_all("SELECT * FROM tbl_config WHERE `key`='$k'");
  if ($xx) {
   $this->db->query("UPDATE tbl_config SET `value`='$v' WHERE `key`='$k'");
  } else {
   $this->db->query("INSERT INTO tbl_config(`key`,`value`) VALUES ('$k','$v')");
  }
 }
 
 function register($singular,$plural = '') {
  $p = $plural ? $plural : $singular . 's';
  $this->registry[$singular] = $singular;
  $this->registry[$p] = $singular;
 }
 
 function register_subtype($base,$subtype) {
  if (! isset($this->registry[$base])) {
   trigger_error('Subtype for nonexistent base',E_USER_ERROR);
  }
 
  $this->subtype_registry[$subtype] = $base;
 }
 
 function load_table($object_type) {
  global $FROG;

  $base = $object_type;
  
  if (! isset($this->registry[$object_type])) {
   if (isset($this->subtype_registry[$object_type])) {
    $base = $this->subtype_registry[$object_type];
   } else {
    trigger_error('Unknown object type: ' . $object_type,E_USER_ERROR);
   }
  }

  $x = $this->registry[$base];

  if (is_string($x)) {
   $FROG = $this;
   $f = $this->include_dir . '/tables/' . $x . '.inc';
   if (! file_exists($f)) {
    trigger_error('Missing include file: ' . $f,E_USER_ERROR);
   }
   require_once($f);
   $x = $this->registry[$base];
  }

  if (! is_object($x)) {
   trigger_error('Unknown object type: ' . $object_type,E_USER_ERROR);
  }

  return $x;
 }

 function add_table($n,$nn,$f,$e,$q = null) {
  $t = new frog_table($this,$n,$nn,$f,$e,$q);
  $this->registry[$n] = $t;
  $this->registry[$nn] = $t;
  return($t);
 }

 function new_object($ot, $pv = null,$with_defaults=1) {
  $t = $this->load_table($ot);
  if (class_exists($ot)) {
   $x = new $ot($pv,$with_defaults);
   $x->object_type = $ot;
   $x->base_type = $t->name;
   return($x);
  } else {
   return(new frog_object($this,$ot,$pv,$with_defaults));
  }
 }

 function get_error() {
  if ($this->db_type == 'sqlite') {
   return $this->db->lastErrorMsg();
  } else {
   return mysqli_error($this->db);
  }
 }
 
 function get_all($q) {
  //  echo "<pre>\r\n{$q}\r\n</pre>\r\n"; flush(); exit; // DEBUG
  $result = $this->db->query($q);
  if ($result === FALSE) {
   $msg = 'Database error in get_all: ' . $this->get_error() . '<br/>' .
    "query:\r\n<pre>\r\n$q\r\n</pre>\r\n";
   echo $msg;
   trigger_error($msg,E_USER_ERROR);
  }

  if ($this->db_type == 'sqlite') {
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
  $result = $this->db->query($q);
  if ($result === FALSE) {
   pre_print($this->db);
   $msg = 'Database error in get_answer:' . $this->get_error();
   // . '<br/>' . "query:\r\n<pre>\r\n$q\r\n</pre>\r\n";
   trigger_error($msg,E_USER_ERROR);
  }

  $x = mysqli_fetch_row($result);
  if ($x === FALSE) {
   return(null);
  } else {
   return($x[0]);
  }
 }

 function bulk_load($table,$key,$ids) {
  $a = array();

  $n = count($ids);
  $i = 0;
  while($i < $n) {
   $j = min(900,$n-$i);
   $s = implode(',',array_slice($ids,$i,$j));
   $b = $this->load_where($table,"$key IN ($s)");
   $a = array_merge($a,$b);
   $i += $j;
  }

  return($a);
 }

 function bulk_load_where($table,$w,$key,$ids) {
  $a = array();

  $n = count($ids);
  $i = 0;
  while($i < $n) {
   $j = min(900,$n-$i);
   $s = implode(',',array_slice($ids,$i,$j));
   $b = $this->load_where($table,"$w AND $key IN ($s)");
   $a = array_merge($a,$b);
   $i += $j;
  }

  return($a);
 }

 function load_generic($object_type,$q) {
  return($this->load_table($object_type)->load_generic($q));
 }

 function load($object_type,$id) {
  return($this->load_table($object_type)->load($id));
 }

 function load_light($object_type,$id) {
  return($this->load_table($object_type)->load_light($id));
 }

 function load_all($object_type) {
  return($this->load_table($object_type)->load_all());
 }

 function load_all_light($object_type) {
  return($this->load_table($object_type)->load_all_light());
 }

 function load_where($object_type,$w) {
  return($this->load_table($object_type)->load_where($w));
 }

 function load_where_current($object_type,$w) {
  return($this->load_table($object_type)->load_where_current($w));
 }

 function load_where_light($object_type,$w) {
  return($this->load_table($object_type)->load_where_light($w));
 }

 function load_where_ordered($object_type,$w,$ord) {
  return($this->load_table($object_type)->load_where_ordered($w,$ord));
 }

 function load_where_grouped($object_type,$w,$field) {
  return($this->load_table($object_type)->load_where_grouped($w,$field));
 }

 function load_where_indexed($object_type,$w,$key = null) {
  return($this->load_table($object_type)->load_where_indexed($w,$key));
 }

 function load_the_one_where($object_type,$w) {
  return($this->load_table($object_type)->load_the_one_where($w));
 }

 function count_where($object_type,$w) {
  return($this->load_table($object_type)->count_where($w));
 }

 function delete($object_type,$id) {
  return($this->load_table($object_type)->delete($id));
 }

 function fields($object_type) {
  return($this->load_table($object_type)->fields);
 }

 function field_names($object_type) {
  return($this->load_table($object_type)->field_names());
 }

 function extra_field_names($object_type) {
  return($this->load_table($object_type)->extra_field_names());
 }

 function all_field_names($object_type) {
  return($this->load_table($object_type)->all_field_names());
 }

 function cache_load($object_type) {
  if (! isset($this->cache)) {
   $this->cache = array();
  }

  if (! isset($this->cache[$object_type])) {
   $this->cache[$object_type] = $this->load_all($object_type);
  }

  return($this->cache[$object_type]);
 }

 function javascript_declaration($include_header = 1) {
  if ($include_header) {
   $script = <<<JS
var {$this->name} = Object.create(frog);
{$this->name}.object = Object.create(frog.object);

JS;
  } else {
   $script = '';
  }
  
  $tt = array();
  foreach($this->registry as $k => $v) {
   $x = $this->load_table($k);
   $tt[$x->name] = $x;
  }

  foreach($tt as $k => $x) {
   $script .= $x->javascript_declaration();
  }
  return $script;
 }

 function sanitise($s) {
  return ($this->escape_string($s));
 }

 function get_safe_required_parameter($p) {
  return($this->sanitise(get_required_parameter($p)));
 }

 function get_safe_optional_parameter($p,$d) {
  return($this->sanitise(get_optional_parameter($p,$d)));
 }

 function get_object_parameter($key,$object_type=null) {
  $ot = $object_type;
  if (! $ot) { $ot = $key; }
  $k = ($key ==  '' || $key == 'id') ? 'id' : $key . '_id';
  if (! isset($_REQUEST[$k])) {
   return(false);
  }

  return $this->load($ot,(int) $_REQUEST[$k]);
 }
}

//////////////////////////////////////////////////////////////////////

class frog_table {
 var $parent,$name,$name_plural,$fields,$extra_fields,$subtypes,
  $primary_key,$primary_key_type,$query_string;

 function __construct($p,$n,$nn,$f,$e,$q=null) {
  $this->parent = $p;
  $this->name = $n;
  $this->name_plural = $nn;
  $this->fields = $f;
  $this->subtypes = null;

  // This fake 'foreach' just picks up the first entry in the
  // associative array $f.  There is probably a better way.
  foreach($f as $k => $v) {
   $this->primary_key = $k;
   $this->primary_key_type = $v['type'];
   break;
  }

  $this->extra_fields = $e;
  if ($q) {
   $this->query_string = $q;
  } else {
   $this->query_string = "SELECT * FROM tbl_{$this->name_plural} x ";
  }
 }

 function field_names() {
  $ff = array();
  foreach ($this->fields as $f => $v) {
   $ff[] = $f;
  }
  return($ff);
 }

 function extra_field_names() {
  $ff = array();
  foreach ($this->extra_fields as $f => $v) {
   $ff[] = $f;
  }
  return($ff);
 }

 function all_field_names() {
  $ff = array();
  foreach ($this->fields as $f => $v) {
   $ff[] = $f;
  }
  foreach ($this->extra_fields as $f => $v) {
   $ff[] = $f;
  }
  return($ff);
 }

 function load($id) {
  $x = $this->parent->new_object($this->name,$id);
  if ($x->load()) {
   return($x);
  } else {
   return(false);
  }
 }

 function load_light($id) {
  $x = $this->parent->new_object($this->name,$id);
  if ($x->load_light()) {
   return($x);
  } else {
   return(false);
  }
 }

 function load_generic($q) {
  $n = $this->name;
  $nn = $this->name_plural;
  $xx = $this->parent->get_all($q);
  $yy = array();
  foreach($xx as $x) {
   $t = $n;
   if (isset($x->subtype) &&
       isset($this->parent->subtype_registry[$x->subtype]) &&
       $this->parent->subtype_registry[$x->subtype] == $x->subtype) {
    $t = $x->subtype;
   }
   $y = $this->parent->new_object($t);
   $y->fill_from_record($x);
   $yy[] = $y;
  }

  return($yy);
 }

 function load_all() {
  #  return $this->load_generic('SELECT * FROM view_' . $this->name_plural);
  return $this->load_generic(sprintf($this->query_string,'1=1'));
 }

 function load_all_light() {
  return $this->load_generic('SELECT * FROM tbl_' . $this->name_plural . ' x');
 }

 function load_where($w) {
  #  return $this->load_generic('SELECT * FROM view_' . $this->name_plural . ' WHERE ' . $w);
  return $this->load_generic(sprintf($this->query_string,$w));
 }

 function load_where_light($w) {
  return $this->load_generic('SELECT * FROM tbl_' . $this->name_plural . ' x WHERE ' . $w);
 }

 function load_where_ordered($w,$o) {
  #  return $this->load_generic('SELECT * FROM view_' . $this->name_plural .
   #			     ' WHERE ' . $w . ' ORDER BY ' . $o);
  $s = $this->query_string;
  $p = strpos($s,'ORDER BY');
  if (! ($p === FALSE)) {
   $s = substr($s,0,$p);
  }
  $q = sprintf($s,$w) . ' ORDER BY ' . $o;
  return $this->load_generic($q);
 }

 function load_where_grouped($w,$field) {
  $s = $this->query_string;
  $p = strpos($s,'ORDER BY');
  if (! ($p === FALSE)) {
   $s = substr($s,0,$p);
  }
  $q = sprintf($s,$w) . ' GROUP BY ' . $field;
  return $this->load_generic($q);
 }

 function load_where_indexed($condition,$key = null) {
  $k = $key ? $key : $this->primary_key;
  $yy = $this->load_where($condition);
  $zz = array();
  foreach($yy as $y) {
   $zz[$y->$k] = $y;
  }
  return($zz);
 }

 function load_the_one_where($w) {
  $n = $this->name;
  $nn = $this->name_plural;
  $q = sprintf($this->query_string,$w);
  $xx = $this->parent->get_all($q);
  
  if (count($xx) == 0) {
   trigger_error('Could not find record: ' . $q,E_USER_ERROR);
  }

  $y = $this->parent->new_object($n);
  $y->fill_from_record($xx[0]);
  return($y);
 }

 function count_where($w) {
  #  $q = "SELECT COUNT({$this->primary_key}) FROM view_{$this->name_plural} WHERE $w";
  $q = "SELECT COUNT({$this->primary_key}) FROM tbl_{$this->name_plural} WHERE $w";
  return (int) $this->parent->get_answer($q);
 }

 function delete($id) {
  $i = 0 + $id;
  $q = 'delete from tbl_' . $this->name_plural . ' where id=' . $i;
  $r = $this->parent->db->query($q);
  if ($r === FALSE) {
   trigger_error('Could not delete ' . $this->name .
		 ' from database: ' . $q . ';' . $this->parent->get_error(),E_USER_ERROR);
  }
 }

 function javascript_declaration() {
  $p = $this->parent->name;
  $n = $this->name;
  $fields = json_encode($this->fields);

  $js = <<<JS
{$p}.{$n} = Object.create({$p}.object);
{$p}.{$n}.object_type = '$n';
{$p}.{$n}.key = '{$this->primary_key}';
{$p}.{$n}.fields = $fields;

JS;

 return($js);
 }
}

//////////////////////////////////////////////////////////////////////

class frog_object {
 var $parent, $object_type, $base_type, $id, $title, $name;
  var $prefix, $new_object_marker, $new_object_number, $primary_key, $primary_key_name, 
      $extra, $changed;

 function __construct($p,$ot,$id = null,$with_defaults=1) {
  $this->parent = $p;
  $this->object_type = $ot;
  $this->extra = new stdClass();
  $m = $p->load_table($ot);
  $this->base_type = $m->name;
  $pk = $m->primary_key;
  if ($id) {
   $this->$pk = $id;
  }
  foreach($m->fields as $k => $v) {
   if ($k != $pk) {
    if (isset($v['default']) && $with_defaults) {
     $this->$k = $v['default'];
    } else {
     $this->$k = null;
    }
   }
  }
 }

 function descriptor() {
  if (isset($this->title) && $this->title) {
   return($this->title);
  } elseif (isset($this->name) && $this->name) {
   return($this->name);
  } elseif ($this->id) {
   return("{$this->object_type}[{$this->id}]");
  } else {
   return "New $this->object_type";
  }
 }

 function strip_parent() {
  $x = clone($this);
  unset($x->parent);
  foreach($x as $k => $v) {
   if ($x->$k instanceof frog_object) {
    $x->$k = $x->$k->strip_parent();
   }
  }
  return $x;
 }
 
 function dump() {
  var_dump($this->strip_parent());
 }
 
 function fill_from_record($x) {
  if (! is_object($x)) {
   trigger_error('In fill_from_record: not an object',E_USER_ERROR);
  }

  $m = $this->parent->load_table($this->object_type);

  foreach($m->fields as $k => $v) {
   if (isset($x->$k)) { 
    if ($v['type'] == 'integer') {
     $this->$k = (int) $x->$k;
    } elseif($v['type'] == 'boolean') {
     $this->$k = $x->$k ? 1 : 0;
    } else {
     $this->$k = $x->$k;
    }
   } else {
    $this->$k = null;
   }
  }

  foreach($m->extra_fields as $k => $v) {
   if (isset($x->$k)) {
    if ($v['type'] == 'integer') {
     $this->$k = (int) $x->$k;
    } elseif($v['type'] == 'boolean') {
     $this->$k = $x->$k ? 1 : 0;
    } else {
     $this->$k = $x->$k;
    }
   } else {
    $this->$k = null;
   }
  }
 }

 function load() {
  $m = $this->parent->load_table($this->object_type);
  $pk = $m->primary_key;
  if (! (isset($this->$pk) && $this->$pk)) {
   trigger_error("No $pk set when loading {$this->object_type} object from database",
		 E_USER_ERROR);
  }
  $pv = $this->$pk;

  if ($m->primary_key_type == 'text') {
   $pv = "'$pv'";
  } else {
   $pv = (int) $pv;
  }

  $q = sprintf($m->query_string,'x.' . $pk . '=' . $pv);

  $s = $this->parent->db->query($q);

  if ($s === FALSE) {
   trigger_error('Error loading ' . $this->object_type . ': '
    . $this->parent->get_error(),E_USER_ERROR);
  }
  if ($this->parent->db_type == 'sqlite') {
   $r = $s->fetchArray(SQLITE3_ASSOC);
   if (is_array($r)) {
    $r = (object) $r;
   } else {
    $r = null;
   }
  } else {
   $r = mysqli_fetch_object($s);
  }
  if ($r) {
   foreach($m->fields as $k => $v) {
    if ($v['type'] == 'integer') {
     $rk = isset($r->$k) ? ($r->$k) : null;
     if ($rk === null) {
      $this->$k = null;
     } else {
      $this->$k = (int) $rk;
     }
    } elseif($v['type'] == 'boolean') {
     $this->$k = $r->$k ? 1 : 0;
    } else {
     $this->$k = $r->$k;
    }
   }
   foreach($m->extra_fields as $k => $v) {
    if ($v['type'] == 'integer') {
     $this->$k = (int) $r->$k;
    } elseif($v['type'] == 'boolean') {
     $this->$k = $r->$k ? 1 : 0;
    } else {
     $this->$k = $r->$k;
    }
   }
   return(true);
  } else {
   return(false);
  }
 }
 
 function load_light() {
  if (! $this->id) {
   trigger_error('No id set when loading object from database',E_USER_ERROR);
  }
  
  $m = $this->parent->load_table($this->object_type);
  $pk = $m->primary_key;
  $pv = $this->$pk;
  
  if ($m->primary_key_type == 'text') {
   $pv = "'$pv'";
  }
  
  $q =
   'SELECT * FROM tbl_' . $m->name_plural .
   ' x WHERE x.' . $pk . '=' . $pv;
  
  $s = $this->parent->db->query($q);
  
  if ($s === FALSE) {
   trigger_error('Error loading ' . $this->object_type . ': ' .
		 $this->parent->get_error(),E_USER_ERROR);
  }
  $r = mysqli_fetch_object($s);
  if ($r) {
   foreach($m->fields as $k => $v) {
    $this->$k = $r->$k;
   }
   return(true);
  } else {
   return(false);
  }
 }
 
 function load_where($w) {
  global $is_error;
  $is_error = false;
  $eh = set_error_handler("ignore_errors");
  $x = $this->parent->load_table($this->object_type)->load_the_one_where($w);
  foreach($this->parent->all_field_names($this->object_type) as $f) {
   $this->$f = $x->$f;
  }
  set_error_handler($eh);
  $x = $is_error;
  $is_error = false;
  return(! $x);
 }
 
 function load_link($n, $force = false) {
  if (isset($this->$n) && $this->$n && ! $force) {
   return($this->$n);
  }

  $m = $this->parent->load_table($this->object_type);
  
  $nid = $n . '_id';
  if (isset($m->fields[$nid])) {
   $f = $m->fields[$nid];
   $i = $this->$nid;
   if ($i) {
    $x = $this->parent->new_object($f['link'],$i);
    if ($x->load()) {
     $this->$n = $x;
     return($x);
    } else {
     $this->$n = null;
     return(null);
    }
   } else {
    $this->dump(); 
    trigger_error('Cannot load link: no ' . $nid . ' recorded');
   }
  } else {
   trigger_error('Cannot load link: no ' . $nid . ' field in ' . $this->object_type);
  }
 }
 
 function load_links() {
  $m = $this->parent->load_table($this->object_type);
  
  foreach($m->fields as $k => $v) {
   if (isset($v['link'])) {
    $u = $v['link'];
    if (substr($k,-3) == '_id') {
     $kl = substr($k,0,strlen($k)-3);
    } else {
     $kl = $k . '_link';
    }
    $t = $v['type'];
    if ($this->$k) {
     $x = $this->parent->new_object($u,$this->$k);
     $b = $x->load(); 
     if ($b) {
      $this->$kl = $x;
     }
    }
   }
  }
 }
 
 function insert() {
  $m = $this->parent->load_table($this->object_type);
  $n = 'tbl_' . $m->name_plural;
  $pk = $m->primary_key;
  
  if ($pk != 'id' && (! $this->$pk)) {
   trigger_error('Could not insert ' . $this->object_type .
		 ' in database: primary key is not autoincrement ' . 
		 ' so must be supplied',E_USER_ERROR);   
  }
  
  $q = "INSERT INTO $n (";
  foreach($m->fields as $k => $v) {
   if ($k != 'id' && $k != 'update_time' && $k != 'timestamp') {
    $q .= '`' . $k . '`,';
   }
  }
  $q = substr($q,0,strlen($q)-1) . ') VALUES (';
  
  foreach($m->fields as $k => $v) {
   if ($k != 'id' && $k != 'update_time' && $k != 'timestamp') {
    $t = $v['type'];
    $x = $this->$k;
    
    if ($x === null) {
     $s = 'NULL';
    } elseif ($t == 'integer') {
     $s = (int) $x;
    } elseif ($t == 'float') {
     $s = (float) $x;
    } elseif ($t == 'boolean') {
     $s = $x ? 1 : 0;
    } elseif ($t == 'date' && ($x == '0000-00-00' || ! $x)) {
     $s = 'NULL';
    } elseif ($t == 'time' && ! $x) {
      $s = 'NULL';
    } else {
     $s = "'" . $this->parent->escape_string($x) . "'";
    }
    
    $q .= $s . ',';
   }
  }
  $q = substr($q,0,strlen($q)-1) . ')';
  
  $s = $this->parent->db->query($q);
  
  if($s === FALSE) {
   trigger_error('Could not insert ' . $this->object_type . ' in database: ' .
		 $this->parent->get_error() . "<br/>\r\n" . $q,E_USER_ERROR);
  }
  
  if ($pk == 'id') {
   if ($this->parent->db_type == 'sqlite') {
    $this->id = $this->parent->db->lastInsertRowID();
   } else {
    $this->id = mysqli_insert_id($this->parent->db);
   }
  }
 }
 
 /* The update() method updates the database row for this object
  * with values taken from this object.  
  *
  * One point to note is that some fields in this object may not be
  * set; in that case, the update() method will make no attempt to
  * modify the corresponding columns in the database.
  */
 function update() {
  $m = $this->parent->load_table($this->object_type);
  $n = 'tbl_' . $m->name_plural;
  $pk = $m->primary_key;
  $q = "UPDATE $n SET ";
  foreach($m->fields as $k => $v) {
   if ($k != $pk && $k != 'update_time'  && $k != 'timestamp' && isset($this->$k)) {
    $t = $v['type'];
    $x = $this->$k; 
    
    if ($x === null) {
     $s = 'NULL';
    } elseif ($t == 'integer') {
     $s = (int) $x;
    } elseif ($t == 'float') {
     $s = (float) $x;
    } elseif ($t == 'boolean') {
     $s = $x ? 1 : 0;
    } elseif ($t == 'date' && ($x == '0000-00-00' || ! $x)) {
     $s = 'NULL';
    } elseif ($t == 'time' && ! $x) {
      $s = 'NULL';
    } else {
     $s = "'" . $this->parent->escape_string($x) . "'";
    }
    
    $q .= '`' . $k . '`=' . $s . ',';
   }
  }
  
  $pv = $this->$pk;
  
  if ($m->primary_key_type == 'text') {
   $pv = "'$pv'";
  }
  
  $q = substr($q,0,strlen($q)-1) . " WHERE $pk=$pv";
  
  $s = $this->parent->db->query($q);
  
  if($s === FALSE) {
   trigger_error('Could not update ' . $this->object_type .
		 ' in database: ' . $this->parent->get_error() .
   '<br/><pre>' . $q . '</pre>',E_USER_ERROR);
  }
 }
 
 function save() {
  $m = $this->parent->load_table($this->object_type);
  $pk = $m->primary_key;
  
  if ($this->$pk) {
   $this->update();
  } else {
   $this->insert();
  }
 }
 
 function delete($ignore_errors=0) {
  if (!$this->id) {
   trigger_error('No id specified in deletion query',E_USER_ERROR);
  }
  
  $m = $this->parent->load_table($this->object_type);
  $n = 'tbl_' . $m->name_plural;
  
  $i = (int) $this->id;
  $q = 'delete from ' . $n . ' where id=' . $i;
  $r = $this->parent->db->query($q);
  if ($r === FALSE && ! $ignore_errors) {
   trigger_error('Could not delete ' . $this->name .
		 ' from database: ' . $q . ';' . $this->parent->get_error(),E_USER_ERROR);
  }
 }
 
 function delete_linked($other_table_name,$link_field) {
  if ($this->id) {
   $table = $this->parent->load_table($other_table_name);
   
   $q = "DELETE FROM tbl_{$table->plural} WHERE {$link_field}={$this->id}";
   $r = $this->parent->db->query($q);
   if ($r === FALSE) {
    trigger_error('Could not delete linked entries: ' . $q . ';' . $this->parent->get_error(),E_USER_ERROR);
   }
  }
 }
 
 function is_empty() {
  $ff = $this->parent->load_table($this->object_type)->fields;
  foreach ($ff as $f => $v) {
   $s = $this->$f;
   $d = null;
   if (isset($v['default'])) { $d = $v['default']; }
   if ($f != 'id' && $this->$f && $this->$f != $d) { return(false); }
  }
  return(true);
 }
 
 function fill_from_request($prefix = '',$force_boolean=0,$fields = null) {
  $changed = 0;
  $ff = $this->parent->load_table($this->object_type)->fields;

  if ($fields) {
   $ff0 = $ff;
   $ff = array();
   foreach ($fields as $f) {
    if (isset($ff0[$f])) {
     $ff[$f] = $ff0[$f];
    }
   }
  }

  foreach ($ff as $f => $v) {
   $t = $v['type'];
   $pf = $prefix . $f;
   if ($f != 'id' && isset($_REQUEST[$pf])) { 
    $x = $_REQUEST[$prefix . $f];
    
    if ($t == 'integer') {
     $x = (int) $x;
    } elseif ($t == 'float') {
     $x = (float) $x;
    } elseif ($t == 'boolean') {
     $x = $x ? 1 : 0;
    }
    
    if (! isset($this->$f) || $this->$f != $x) { $changed = 1; }
    $this->$f = $x;
   } elseif($force_boolean && $t=='boolean') {
    $x=0;
    if (! isset($this->$f) || $this->$f != $x) { $changed = 1; }
    $this->$f = $x;
   }	 
  }
  return($changed); 
 }
 
 function fill_list_from_request($f,$parent_key,$parent_id) {
  $t = $this->parent->load_table($f);
  $singular = $t->name;
  $plural = $t->name_plural;
  $a = array();
  $nk = "num_$plural";
  $n = isset($_REQUEST[$nk]) ? 0 + $_REQUEST[$nk] : 0;
  
  for ($i = 0; $i < $n ; $i++) {
   $obj = $this->parent->new_object($singular);
   foreach ($t->fields as $f => $v) {
    $k = "{$singular}_{$i}_$f";
    if (isset($_REQUEST[$k])) {
     $obj->$f = stripslashes($_REQUEST[$k]);
    }
   }
   if ($obj->is_empty()) {
    if ($obj->id) { 
     $obj->delete(1); 
    } 
   } else {
    $obj->$parent_key = $parent_id;
    $a[] = $obj;
   }
  }
  
  $this->$plural = $a;
 }
 
 function extend_list($f,$n = 1,$p = null) {
  // Add some empty objects to a list
  $t = $this->parent->load_table($f);
  $singular = $t->name;
  $plural = $p ? $p : $t->name_plural;
  if (! isset($this->$plural)) {
   $this->$plural = array();
  }
  $a = $this->$plural;
  
  if ($a && ($singular == 'book')) {
   $last_rec_type = 
    $a[count($a)-1]->recommendation_type;
  } else {
   $last_rec_type = '';
  }
  for ($i = 0; $i < $n; $i++) {
   $b = $this->parent->new_object($singular);
   $b->new_object_number = $i;
   if ($singular == 'book') {
    $b->recommendation_type = $last_rec_type;
   }
   $a[] = $b;
  }
  $this->$plural = $a;
 }
 
 function convert_latex() {
  $ff = $this->parent->load_table($this->object_type)->fields;
  foreach($ff as $f => $v) {
   if (isset($v['latex']) && $v['latex']) {
    $t = $this->$f;
    if (isset($v['latex_prefix'])) {
     $t = $v['latex_prefix'] . $t;
    }
    if (isset($v['latex_suffix'])) {
     $t = $t . $v['latex_suffix'];
    }
    $g = $f . '_html';
    $this->$g = latex_to_html($t);
   }
  }
 }
 
 function prune($fields = null) {
  $tb = $this->parent->load_table($this->object_type);
  $ff = $fields ? $fields : $tb->field_names();
  $x = new stdClass();
  foreach($ff as $f) {
   $x->$f = $this->$f;
   if (isset($tb->fields[$f])) {
    $gg = $tb->fields[$f];
    if ($gg['type'] == 'integer' || $gg['type'] == 'boolean') {
     $x->$f = (int) ($x->$f);
    }
   }
  }
  
  return($x);
 }
 
 function json($fields = null) {
  return(json_encode($this->prune($fields)));
 }
 
 function set_prefix($n = '') {
  $nn = $n ? $n : $this->object_type;
  
  if ($this->id) {
   $this->prefix = "{$nn}_{$this->id}";
   $this->new_object_marker = '';
  } else {
   $this->prefix = "add_{$nn}_{$this->new_object_number}";
   $this->new_object_marker = 
    '<input type="hidden" value="1" name="' . $this->prefix . '"/>';
  }
  
  return $this->prefix;
 }
 
 function html_entities() {
  $x = new stdClass();
  $m = $this->parent->load_table($this->object_type);

  foreach($m->fields as $k => $v) {
   if (isset($this->$k)) { 
    if ($v['type'] == 'integer') {
     $x->$k = (int) $this->$k;
    } elseif($v['type'] == 'boolean') {
     $x->$k = $this->$k ? 1 : 0;
    } else {
     $x->$k = htmlentities($this->$k);
    }
   } else {
    $x->$k = null;
   }
  }

  foreach($m->extra_fields as $k => $v) {
   if (isset($this->$k)) { 
    if ($v['type'] == 'integer') {
     $x->$k = (int) $this->$k;
    } elseif($v['type'] == 'boolean') {
     $x->$k = $this->$k ? 1 : 0;
    } else {
     $x->$k = htmlentities($this->$k);
    }
   } else {
    $x->$k = null;
   }
  }

  return $x;
 }

 function remover_box($level = 1) {
  $u = $this->parent->html->icons_url;
  $n = $this->parent->name;
  
  return '<input type="checkbox" name="remove_' . $this->prefix . 
   '" onclick="frog.checkbox_hide_siblings(this,' . $level . ')"/>' .
   '<img alt="remove" src="' . $u . '/bin.png"/>';
 }
 
 function wrap_remover($h,$level = 1) {
  $id = $this->prefix . '_wrapper';
  return '<tr><td id="' . $id . '">' . $h . '</td><td valign="top">' .
   $this->remover_box($level) . '</td></tr>';
 }
}

?>
