<?php

//////////////////////////////////////////////////////////////////////

function dont_cache() {
  ////////////////////////////////////////////////////////////
  // Prevent caching:
  
  // Date in the past
  header("Expires: Mon, 26 Jul 1997 05:00:00 GMT");
  // always modified
  header("Last-Modified: " . gmdate("D, d M Y H:i:s") . " GMT");

  // These lines seem redundant 
  // HTTP/1.1
  //header("Cache-Control: no-store, no-cache, must-revalidate");
  //header("Cache-Control: post-check=0, pre-check=0", false);
  // HTTP/1.0

  // This used to say Pragma: no-cache, but that seems to break 
  // IE versions <= 8.  In any case, PHP seems to set
  // Pragma: no-cache automatically anyway.  Here we set 
  // Pragma to be empty to suppress that automatic behaviour.
  // I think that setting Expires and Last-Modified is enough
  // to prevent real caching by IE, while permitting it to save a 
  // momentary temporary copy that it can pass to the PDF viewer.
  header("Pragma: ");
  ////////////////////////////////////////////////////////////
}

//////////////////////////////////////////////////////////////////////

function is_number($s) {
 return( (! is_null($s)) && 
	 (strspn($s,'0123456789') == strlen($s)));
}

//////////////////////////////////////////////////////////////////////

function starts_with($prefix,$s) {
 $n = strlen($prefix);
 return(strlen($s) >= $n && substr($s,0,$n) == $prefix);
}

//////////////////////////////////////////////////////////////////////

function ends_with($suffix,$s) {
 $n = strlen($suffix);
 return(strlen($s) >= $n && substr($s,-$n) == $suffix);
}
 
//////////////////////////////////////////////////////////////////////

// $start_time and $end_time are like 5:10 or 09:30
// The return value is like array($s,$e), where usually $s represents
// the same time as start_time, but in the format 05:10:00, and 
// similarly for $e.  However:
// * If $start_time is '',0,null,'00:00:00' etc then $s and 
//   $e will both be ''.
// * Otherwise, $e will be nonempty and after $s.  If $end_time
//   is null etc then $e will be one hour after $s.

function fix_times($start_time,$end_time) {
 $hms = explode(':',$start_time);
 $hms[] = 0; $hms[] = 0; 
 $start_hour = (int) $hms[0];
 $start_min  = (int) $hms[1];

 if ($start_hour < 0 ) { $start_hour = 0; }
 if ($start_hour > 23) { $start_hour = 23; }
 if ($start_min  < 0 ) { $start_min = 0; }
 if ($start_min  > 59) { $start_min = 59; }

 $hms = explode(':',$end_time);
 $hms[] = 0; $hms[] = 0; 
 $end_hour = (int) $hms[0];
 $end_min  = (int) $hms[1];

 if ($end_hour < 0 ) { $end_hour = 0; }
 if ($end_hour > 23) { $end_hour = 23; }
 if ($end_min  < 0 ) { $end_min = 0; }
 if ($end_min  > 59) { $end_min = 59; }

 if ($start_hour) {
  if ($end_hour) {
   if (($end_hour < $start_hour) ||
       ($end_hour == $start_hour && $end_min < $start_min)) {
    $end_hour = $start_hour;
    $end_min = $start_min;
   }
  } else {
   $end_hour = $start_hour + 1;
   $end_min = $start_min;
  } 
  $s = sprintf("%02d:%02d:00",$start_hour,$start_min);
  $e = sprintf("%02d:%02d:00",$end_hour,  $end_min);
 } else {
  $s = '';
  $e = '';
 }

 return(array($s,$e));
}
 
//////////////////////////////////////////////////////////////////////

function quote_quotes($s) {
 $t = "";
 $i = 0;
 while($j = strpos($s,"'",$i)) {
  $t = $t . substr($s,$i,$j-$i) . "\\'";
  $i = $j+1;
 }
 $t = $t . substr($s,$i);
 return($t);
}

//////////////////////////////////////////////////////////////////////

function fill_template($template,$x) {
 $i = 0;
 $l = strlen($template);
 $s = "";

 while($j = strpos($template,"\\info{",$i)) {
  $s = $s . substr($template,$i,$j-$i);
  if ($k = strpos($template,"}{}",$j+6)) {
   $key = substr($template,$j+6,$k-$j-6);
   $s = $s . "\\info{" . $key . "}{" . $x->$key . "}";
   $i = $k+3;
  } else {
   $s = $s . substr($template,$j+6,$l-$j-6);
   $i = $l;
  }
 }
 $s = $s . substr($template,$i,$l-$i);
 return($s);
}

//////////////////////////////////////////////////////////////////////

function get_required_parameter($p) {
 if (isset($_REQUEST[$p])) {
  return($_REQUEST[$p]);
 } else {
  trigger_error("Missing parameter: $p");
  exit;
 }
}

//////////////////////////////////////////////////////////////////////

function get_optional_parameter($p,$d) {
 if (isset($_REQUEST[$p])) {
  return($_REQUEST[$p]);
 } else {
  return($d);
 }
}

//////////////////////////////////////////////////////////////////////

function get_restricted_parameter($p,$v,$d) {
 if (isset($_REQUEST[$p])) {
  $x = $_REQUEST[$p];
  foreach($v as $y) {
   if ($x == $y) { return($x); }
  }
 }
 return($d);
}

//////////////////////////////////////////////////////////////////////

function sanitise($s,$frog) {
 return($frog->sanitise($s));
}

//////////////////////////////////////////////////////////////////////

function get_safe_required_parameter($p,$frog) {
 return($frog->sanitise(get_required_parameter($p)));
}

//////////////////////////////////////////////////////////////////////

function get_safe_optional_parameter($p,$d,$frog) {
 return($frog->sanitise(get_optional_parameter($p,$d)));
}

//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////

function latex_to_html($tex_source,$tth_opts = '-r -u') {
 global $cfg;

 $n = sprintf("%d%d",rand(),time());
 $f = $cfg->tth_dir . '/' . $n;
 if ($cfg->exec_allowed) {
  $ts = <<<TEXT
\documentclass{article}
\\newcommand{\N}{{\bf N}}
\\newcommand{\Z}{{\bf Z}}
\\newcommand{\Q}{{\bf Q}}
\\newcommand{\R}{{\bf R}}
\\newcommand{\C}{{\bf C}}
\\newcommand{\mathbb}[1]{{\bf #1}}
\begin{document}
$tex_source
\\end{document}

TEXT;

  file_put_contents($f,$ts);

  $cmd = "{$cfg->tth_program} {$tth_opts} < " .  $f ." > {$f}.html";

  exec($cmd);
  if (file_exists($f . '.html')) {
   $h = file_get_contents($f . '.html');
   @ unlink($f);
   @ unlink($f . '.html');
   // fix bug whereby blank lines are appended to start
   if(strpos($h,"\n\n\n\n\n \n")!==false) {
    $h = str_replace("\n\n\n\n\n \n","",$h); 
   }
   return $h;
  } else {
   //   trigger_error('TtH failed: command = ' . $cmd);
   return '';
  }
 } else {
  file_put_contents($f,$tex_source);
  $tth_url = 'http://' . $_SERVER['SERVER_NAME'] . '/cgi-bin/tth.pl';
  $url = $tth_url . '?tex_file=' . $n . '&tth_opts=' . 
   rawurlencode($tth_opts);
  $h = file_get_contents($url);
  // fix bug whereby blank lines are appended to start
  if(strpos($h,"\n\n\n\n\n \n")!==false) {
   $h = str_replace("\n\n\n\n\n \n","",$h); 
  }
  return($h);
 }
}

//////////////////////////////////////////////////////////////////////

function latex_to_pdf($tex_dir,$tex_file,$display = 1) {
 global $cfg;

 chdir($tex_dir);
 $url = '';
 $cmd = '';

 if ($cfg->exec_allowed) {
  $cmd = "{$cfg->latex_program} \\nonstopmode\\input{" .  $tex_file ."} > {$tex_file}.out 2>&1";
  $status = exec($cmd);
 } else {
  $url = "{$cfg->latex_url}?tex_dir={$tex_dir}&tex_file={$tex_file}";
  $status = (int) file_get_contents($url);
 }

 
 if ($status == 0) {
	if($display) {
	  header('Content-Type: application/pdf');
	  header("Content-Disposition: inline; filename={$tex_file}.pdf");
	  dont_cache();

	  readfile($tex_file .'.pdf');
	}
 } else {
  if (file_exists($tex_file . '.log')) {
   $msg = <<<HTML
PDFLaTeX returned with exit status $status.
Output was as follows:
<pre>

HTML;

   $msg .= file_get_contents("{$tex_file}.log");

   $msg .= <<<HTML
</pre>
tex_dir = {$tex_dir}<br/>
tex_file = {$tex_file}<br/>
latex url = $url<br/>
cmd = $cmd<br/>

HTML;

   error_page($msg,'LaTeX errors');
  } else {
   $msg = <<<HTML
PDFLaTeX returned with exit status $status.  No log file was produced.<br/>
tex_dir = {$tex_dir}<br/>
tex_file = {$tex_file}<br/>
latex url = $url<br/>
cmd = $cmd<br/>

HTML;

   error_page($msg,'LaTeX errors');
  }
 } 
}

//////////////////////////////////////////////////////////////////////

function pre_print($x,$s = '') {
 echo "<hr/>";
 if ($s) { 
  echo "<b>$s</b><br/>\n";
 }
 echo "<pre>\n";
 print_r($x);
 echo "</pre>\n";
}

//////////////////////////////////////////////////////////////////////

function request_pane() {
 echo <<<HTML
<div class="tabbertab">
<h2>Request</h2>
HTML;

pre_print($_REQUEST);

 echo <<<HTML
</div>

HTML;

}

//////////////////////////////////////////////////////////////////////

function rmdir_rf($dir) {
 $d = $dir;
 if (substr($d,-1) != '/') { $d = $d . '/'; }
 $dir_contents = scandir($d);
 foreach ($dir_contents as $item) {
  if (is_dir($d.$item) && $item != '.' && $item != '..') {
   rmdir_rf($d.$item.'/');
  }
  elseif (file_exists($d.$item) && $item != '.' && $item != '..') {
   unlink($d.$item);
  }
 }
 rmdir($dir);
}

//////////////////////////////////////////////////////////////////////

function error_page($msg,$title = 'Error',$back = null) {
 global $cfg;
 if ($back) {
  $b = $back;
 } else {
  $b = 'javascript: history.go(-1)';
 }

 echo <<<HTML
<html>
<head>
<title>$title</title>
</head>
<body>
<h1>
$title
</h1>

$msg
<br/>
<a href="$b">Back</a>
</body>
</html>

HTML;
}

//////////////////////////////////////////////////////////////////////

function ignore_errors() {
 return(true);
}

//////////////////////////////////////////////////////////////////////

function random_password($speakable=1){ 

 srand((double)microtime()*1000000); 

 $vowels = array("a", "e", "i", "o", "u"); 
 $cons = array("b", "c", "d", "g", "h", "j", "k", "l", "m", "n", "p", "r", "s", "t", "v", "w", "tr", 
	       "cr", "br", "fr", "th", "dr", "ch", "ph", "wr", "st", "sp", "sw", "pr", "sl", "cl"); 
     
 $num_vowels = count($vowels); 
 $num_cons = count($cons); 

 $password = '';

 for($i = 0; $i < 6; $i++){ 
  $password .= $cons[rand(0, $num_cons - 1)];
  if($speakable) {
	$password .= $vowels[rand(0, $num_vowels - 1)];
  }
 } 
 
 return substr($password, 0, 6); 
} 

//////////////////////////////////////////////////////////////////////

if (!function_exists('file_put_contents')) {
 function file_put_contents($filename, $data, $respect_lock = true)
  {
   // Open the file for writing
   $fh = @fopen($filename, 'w');
   if ($fh === false) {
    return false;
   }
   
   // Check to see if we want to make sure the file is locked before we write to it
   if ($respect_lock === true && !flock($fh, LOCK_EX)) {
    fclose($fh);
    return false;
   }
   
   // Convert the data to an acceptable string format
   if (is_array($data)) {
    $data = implode('', $data);
   } else {
    $data = (string) $data;
   }
   
   // Write the data to the file and close it
   $bytes = fwrite($fh, $data);
   
   // This will implicitly unlock the file if it's locked
   fclose($fh);

   return $bytes;
  }
}

//////////////////////////////////////////////////////////////////////

function parse_name($s) {
 $x = new stdClass();
 $x->title = '';
 $x->forename = '';
 $x->surname = '';

 $nn = preg_split("/\s+/",$s);

 $i = count($nn);
 while ($i && trim($nn[$i-1]) == '') { $i--; }

 if ($i) {
  $x->surname = $nn[--$i];

  if ($i) {
   $j = 0;
   if ($nn[0] == 'Dr' || $nn[0] == 'Prof' || 
       $nn[0] == 'Mr' || $nn[0] == 'Mrs' || $nn[0] == 'Ms' ||
       $nn[0] == 'Miss') {
    $x->title = $nn[0];
    $j = 1;
   } elseif ($nn[0] == 'Professor') {
    $x->title = 'Prof';
    $j = 1;
   }

   $f = '';
   while ($j < $i) {
    $f .= ' ' . $nn[$j++];
   }
   if ($f) { $f = substr($f,1); }
   $x->forename = $f;
  }
 }

 if (substr($x->forename,-1) == ',') {
  $t = substr($x->forename,0,-1);
  $x->forename = $x->surname;
  $x->surname = $t;
 }

 return $x;
}

//////////////////////////////////////////////////////////////////////

function dmy($date) {
 return 
  substr($date,8,2) . '/' .
  substr($date,5,2) . '/' .
  substr($date,2,2);
}

function dmyy($date) {
 return 
  substr($date,8,2) . '/' .
  substr($date,5,2) . '/' .
  substr($date,0,4);
}

//////////////////////////////////////////////////////////////////////

function trim_time($t) {
 // Remove the seconds from a time, eg 12:10:00 -> 12:10
 return (strlen($t) > 5) ? substr($t,0,5) : $t;
}

function trim_title($s) {
 // Strip non-academic titles
 $t = trim($s);
 $uu = array('Mr ','Mrs ','Miss ','Ms ');
 foreach ($uu as $u) {
  $m = strlen($u);
  if (strlen($t) >= $m && substr($s,0,$m) == $u) {
   $t = substr($t,$m);
  }
 }
 return(trim($t));
}

//////////////////////////////////////////////////////////////////////

function log_write($s,$header = 1) {
 global $cfg;
 $fh = @fopen($cfg->log_file,'a');
 if ($fh === false) {
  return false;
 }
 
 if ($header) {
  $t = "============================================================\n";
  $t .= date('Y-m-d H:i:s') . "\n";
  $t .= $_SERVER['PHP_SELF'] . "\n\n";
  fwrite($fh,$t);
 }

 fwrite($fh,$s);
 fclose($fh);
}

function log_dump($x,$header = 0, $caption = '') {
 ob_start();
 var_dump($x);
 $a=ob_get_contents();
 ob_end_clean();
 if ($caption ) { $a = "$caption:\n$a"; }
 log_write($a);
}

$cfg = new stdClass();

$cfg->this_url        = 'https://' . $_SERVER['HTTP_HOST'] . 
                         $_SERVER['SCRIPT_NAME']; 
$cfg->parent_url      = 'http://' . $_SERVER['HTTP_HOST'] . 
                         dirname($_SERVER['SCRIPT_NAME']); 
$cfg->grandparent_url = 'http://' . $_SERVER['HTTP_HOST'] . 
                         dirname(dirname($_SERVER['SCRIPT_NAME'])); 

$cfg->js_url = 'js';
$cfg->css_url = 'css';

$tabber_script = <<<HTML
 <script type="text/javascript" src="{$cfg->js_url}/tabber.js"></script>
 <link rel="stylesheet" href="{$cfg->css_url}/tabber_inner.css" TYPE="text/css" MEDIA="screen">

HTML;

$autosuggest_script = <<<HTML
 <script type="text/javascript" src="{$cfg->js_url}/autosuggest.js"></script>
 <link rel="stylesheet" href="{$cfg->css_url}/autosuggest.css" TYPE="text/css" MEDIA="screen">

HTML;

$cfg->date_is_current = <<<SQL
 IF((x.start_date IS NULL OR
     x.start_date='0000-00-00' OR
     x.start_date <= CURDATE()) AND
    (x.end_date IS NULL OR
     x.end_date='0000-00-00' OR
     x.end_date >= CURDATE()),1,0)
SQL;

$cfg->date_is_past = <<<SQL
 IF((x.end_date IS NOT NULL AND
     x.end_date <> '0000-00-00' AND
     x.end_date <= CURDATE()),1,0)
SQL;

$cfg->time_is_current = <<<SQL
IF(
 (x.start_time IS NULL OR
  x.start_time='0000-00-00 00:00:00' OR
  x.start_time <= NOW()) AND
 (x.end_time IS NULL OR
  x.end_time='0000-00-00 00:00:00' OR
  x.end_time >= NOW()),1,0)
SQL;

/* When we load all records from a small and seldom-changing table,
 * we store the result in $cfg->cache.
 */

//////////////////////////////////////////////////////////////////////

function make_index($a,$key) {
 $b = array();
 foreach($a as $x) {
  if ($x->$key) {
   $b[$x->$key] = $x;
  }
 }
 return($b);
}

//////////////////////////////////////////////////////////////////////

function get_latlng($url) {
 $x = new stdClass();
 $x->lat = null;
 $x->lng = null;
 $x->ok = true;
 $x->error = null;

 if (! trim($url)) {
  $x->ok = false;
  $x->error = "Empty URL";
  return $x;
 }
 
 $ch = curl_init();
 curl_setopt($ch, CURLOPT_URL, $url);
 curl_setopt($ch, CURLOPT_FOLLOWLOCATION, true);
 curl_setopt($ch, CURLOPT_HEADER, true);
 curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);
 curl_setopt($ch, CURLOPT_NOBODY, true);
 curl_setopt($ch, CURLOPT_SSL_VERIFYHOST, 0);
 curl_setopt($ch, CURLOPT_SSL_VERIFYPEER, 0);
 
 $response = curl_exec($ch);
 if ($response == false) {
  $x->ok = false;
  $x->error = curl_error($ch);
  return $x;
 }

 $new_url = curl_getinfo($ch, CURLINFO_EFFECTIVE_URL);

 curl_close($ch);

 if (preg_match('/@(-?[0-9]+\.[0-9]+),(-?[0-9]+\.[0-9]+),/',$new_url,$m)) {
  $x->lat = $m[1];
  $x->lng = $m[2];
  return $x;
 } else {
  $x->ok = false;
  $x->error = "Couldn't find lat/lng in URL";
  return $x;
 }
}

?>
