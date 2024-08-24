<?php

class frog_navigator {
 var $frog, 
     $standard_scripts, $standard_body_scripts, $scripts_base_dir, $scripts_base_url, $scripts_colocated = 1, 
     $scripts, $inline_script, $body_scripts, $inline_body_script,
     $standard_styles, $styles_base_dir, $styles_base_url, $styles_colocated = 1, 
     $styles, $inline_style,
     $onload,
     $widgets, $available_widgets, $selectors,
     $frog_script_url, $index_url, $dtd, $default_options;
 
 function __construct($frog) {
  $this->frog = $frog;
  $this->standard_scripts = array();
  $this->standard_body_scripts = array();
  $this->standard_styles = array();
  $this->dtd = "<!DOCTYPE HTML>";
   
  $this->available_widgets = array(
   'tabber' => 1,
   'autosuggest' => 1,
   'calendar' => 1,
   'mathjax' => <<<HTML
  <script>MathJax = { tex : { inlineMath : [['$','$'],['\\\\(','\\\\)']] }};</script>
  <script id="MathJax-script" async
          src="https://cdn.jsdelivr.net/npm/mathjax@3/es5/tex-mml-chtml.js"></script>

HTML
,
   'tinymce' => <<<HTML
<script src="//cdn.tinymce.com/4/tinymce.min.js">
 tinyMCE.init({
  mode : "textareas",
  theme : "modern",
  plugins: "link lists image",
  width : "700px"
 });
</script>

HTML
,
   'pdfjs' => <<<HTML
<script src="https://cdn.jsdelivr.net/npm/pdfjs-dist/build/pdf.js"></script>

HTML
  );

  $this->default_options = array(
				 'widgets' => null,
				 'selectors' => null,
				 'scripts' => null,
				 'inline_script' => null,
				 'body_scripts' => null,
				 'inline_body_script' => null,
				 'styles' => null,
				 'inline_style' => null,
				 'onload' => null
				 );

  foreach($this->default_options as $k => $v) {
   $this->$k = $v;
  }
 }

 function set_options($oo) {
  foreach ($this->default_options as $k => $v) {
   if ($oo && array_key_exists($k,$oo)) {
    $this->$k = $oo[$k];
   }
  }
 }
 
 function header($title,$oo = null) {

  if ($oo) { $this->set_options($oo); }
  
  $this->dont_cache();

  $dtd = $this->dtd;
  $fsu = $this->frog_script_url;
  
  echo <<<HTML
{$dtd}
<html>
<head>
<meta charset="UTF-8" />
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>$title</title>
<script type="text/javascript" src="{$fsu}"></script>

HTML;

  foreach($this->standard_scripts as $s) {
   $this->add_script($s); 
  }

  foreach($this->standard_styles as $s) {
   $this->add_style($s); 
  }

  if ($this->widgets) {
   $widgets_used = array();
   foreach($this->widgets as $w) {
    if (! array_key_exists($w,$this->available_widgets)) {
     trigger_error('Unrecognised widget: ' . $w,E_USER_ERROR);
    }
    $widgets_used[$w] = 1;
   }

   foreach($this->available_widgets as $w => $v) {
    if (array_key_exists($w,$widgets_used)) {
     if ($v === 1 ) {
      $this->add_script($w);
      $this->add_style($w);
     } else {
      echo $v;
     }
    }
   }
  }
  
  if ($this->scripts) {
   foreach($this->scripts as $s) {
    $this->add_script($s);
   }
  }

  if ($this->inline_script) {
   $this->add_inline_script($this->inline_script);
  }
 
  if ($this->styles) {
   foreach($this->styles as $s) {
    $this->add_style($s);
   }
  }
  
  if ($this->inline_style) {
   $this->add_inline_style($this->inline_style);
  }

  $onload = '';
  
  if ($this->onload) {
   $onload = " onload=\"{$this->onload}\"";
  }

  echo <<<HTML
</head>
<body $onload>
<!-- <noscript>
Warning: you appear to have disabled Javascript.  These 
pages are heavily dependent on Javascript,
and many things will not work when Javascript is turned off.
</noscript> -->

<br/>
<div id="main_div">
<br/>

HTML;
 }

 function footer() {
  echo <<<HTML
</div>

HTML;

  foreach($this->standard_body_scripts as $s) {
   $this->add_script($s); 
  }

  if ($this->body_scripts) {
   foreach($this->body_scripts as $s) {
    $this->add_script($s);
   }
  }

  if ($this->inline_body_script) {
   $this->add_inline_script($this->inline_body_script);
  }

  echo <<<HTML
</body>
</html>
   
HTML;
 }
 
 function dont_cache() {
  ////////////////////////////////////////////////////////////
  // Prevent caching:
  
  // Date in the past
  header("Expires: Mon, 26 Jul 1997 05:00:00 GMT");
  // always modified
  header("Last-Modified: " . gmdate("D, d M Y H:i:s") . " GMT");

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

 function colocated_file_exists($f) {
  $d = dirname($_SERVER['SCRIPT_FILENAME']);
  $df = $d . '/' . $f;
  if (file_exists($df)) {
   return $df;
  } else {
   return null;
  }
 }

 function file_is_url($f) {
  return (
	  (strlen($f) >= 4 && substr($f,0,4) == 'http') ||
	  (strlen($f) >= 2 && substr($f,0,2) == '//')
	  );
 }
 
 function add_script($s) {
  if ($this->file_is_url($s)) {
   $f = null;
   $u = $s;
  } else if ($this->scripts_colocated &&
	     ($cf = $this->colocated_file_exists($s . '.js'))) {
   $f = $cf;
   $u = $s . '.js';
  } else {
   $f = $this->scripts_base_dir . '/' . $s . '.js'; 
   $u = $this->scripts_base_url . '/' . $s . '.js'; 
  }

  if ($f) {
   $u = $u . '?' . filemtime($f);
  }
  
  echo <<<HTML
<script type="text/javascript" src="$u"></script>

HTML;
 }

 function add_inline_script($s) {
  echo <<<HTML
<script type="text/javascript">
$s
</script>

HTML;
 }

 function add_style($s) {
  $cf = '';
  if ($this->file_is_url($s)) {
   $f = null;
   $u = $s;
  } else if ($this->styles_colocated &&
	     ($cf = $this->colocated_file_exists($s . '.css'))) {
   $f = $cf;
   $u = $s . '.css';
  } else {
   $f = $this->styles_base_dir . '/' . $s . '.css'; 
   $u = $this->styles_base_url . '/' . $s . '.css'; 
  }

  if ($f) {
   $u = $u . '?' . filemtime($f);
  }

  echo <<<HTML
<style type="text/css" media="screen">
  @import url($u);
</style>

HTML;
 }

 function add_inline_style($s) {
  echo <<<HTML
<style type="text/css">
$s
</style>

HTML;
 }
}

?>
