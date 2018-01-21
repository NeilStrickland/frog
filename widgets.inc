<?php

/**
 * This file defines a variety of functions that are used to generate
 * chunks of HTML, especially for user interface elements.
 *
 */

function bold($s) { return '<b>' . $s . '</b>'; }
function fade($s) { return '<span class="fade">' . $s . '</span>'; }

function td($s,$width = 0,$opts = null) {
 $attrib = $width ? " width=\"$width\"" : '';
 
 if ($opts) {
  foreach($opts as $k => $v) {
   $attrib .= ' ' . $k . '="' . $v . '"';
  }
 }
 return("<td{$attrib}>$s</td>");
}

function command_td($s,$action,$width = 0,$opts = null) {
 $attrib = ' class="command" onclick="' . $action . '"';
 if ($width) { $attrib .= ' width="' .  $width . '"'; }
 if ($opts) {
  foreach($opts as $k => $v) {
   $attrib .= ' ' . $k . '="' . $v . '"';
  }
 }
 return("<td{$attrib}>$s</td>");
}

function link_td($s,$href,$width = 0,$opts = null) {
 $attrib = ' class="command" onclick="location=\'' . $href . '\'"';
 if ($width) { $attrib .= ' width="' .  $width . '"'; }
 if ($opts) {
  foreach($opts as $k => $v) {
   $attrib .= ' ' . $k . '="' . $v . '"';
  }
 }
 return("<td{$attrib}>$s</td>");
}

function popup_td($s,$href,$width = 0,$opts = null) {
 $attrib = ' class="command" onclick="window.open(\'' . $href . '\')"';
 if ($width) { $attrib .= ' width="' .  $width . '"'; }
 if ($opts) {
  foreach($opts as $k => $v) {
   $attrib .= ' ' . $k . '="' . $v . '"';
  }
 }
 return("<td{$attrib}>$s</td>");
}

function tr($s,$opts=null) {
 $o = '';
 if ($opts) {
	foreach ($opts as $k=>$v) {
		$o .= " $k='$v'";
	}
 }
 return("<tr $o>" . $s . '</tr>');
}

function row() {
 $s = '';
 $aa = func_get_args();
 foreach($aa as $a) {
  $s .= td($a);
 }
 return(tr($s));
}

function zebra_start() {
 global $zebra;
 $zebra = 0;
}

function zebra_row() {
 global $zebra;

 $s = '';
 $aa = func_get_args();
 foreach($aa as $a) {
  $s .= td($a);
 }
 $c = $zebra ? 'lightbg' : 'medbg';
 $zebra = ! $zebra;
 return('<tr class="' . $c . '">' . $s . '</tr>');
}

function zebra_tr($s) {
 global $zebra;
 $c = $zebra ? 'lightbg' : 'medbg';
 $zebra = ! $zebra;
 return('<tr class="' . $c . '">' . $s . '</tr>');
}

function spacer_row() {
 $s = '';
 $aa = func_get_args();
 foreach($aa as $a) {
  $s .= "<td width=\"$a\"></td>";
 }
 return('<tr class="spacer">' . $s . '</tr>');
}

function table($s) {
 return '<table>' . $s . '</table>';
}

function table_start() {
 return <<<HTML
<table>

HTML;
}

function table_end() {
 return <<<HTML
</table>

HTML;
}

//////////////////////////////////////////////////////////////////////

function lnk($href,$text) {
 return('<a href="' . $href . '">' . $text . '</a>');
}

function popup($href,$text) {
 return('<a href="' . $href . '" target="_blank">' . $text . '</a>');
}

function tooltip($text,$tip) {
 return (<<<HTML
<span onmouseover="Tip('$tip')">$text</span>
HTML
);
}

function tooltip_td($text,$tip) {
 return (<<<HTML
<td onmouseover="Tip('$tip')">$text</span>
HTML
);
}

//////////////////////////////////////////////////////////////////////

function icon($f,$frog) {
 return('<img alt="' . $f . '" src="' . $frog->cfg->icons_url . '/' . $f . '.png"/>');
}

function icon_button($f,$action,$frog) {
 return('<img class="link" alt="' . $f . '" src="' . $frog->cfg->icons_url . '/' . $f .
	'.png" onclick="' . $action . '"/>');
}

function icon_link($f,$url,$frog) {
 return('<a href="' . $url . '"><img alt="' . $f .
	'" src="' . $frog->cfg->icons_url . '/' . $f .
	'.png"/></a>');
}

function icon_popup($f,$url,$frog) {
 return('<a target="_blank" href="' . $url . '"><img alt="' . $f .
	'" src="' . $frog->cfg->icons_url . '/' . $f .
	'.png"/></a>');
}

//////////////////////////////////////////////////////////////////////

function clone_button($action,$frog)   { return icon_button('clone'  ,$action,$frog); }
function delete_button($action,$frog)  { return icon_button('delete' ,$action,$frog); }
function display_button($action,$frog) { return icon_button('display',$action,$frog); }
function edit_button($action,$frog)    { return icon_button('edit'   ,$action,$frog); }
function pdf_button($action,$frog)     { return icon_button('pdf'    ,$action,$frog); }
function email_button($action,$frog)   { return icon_button('email'  ,$action,$frog); }

function clone_link($url,$frog)   { return icon_link('clone'  ,$url,$frog); }
function delete_link($url,$frog)  { return icon_link('delete' ,$url,$frog); }
function display_link($url,$frog) { return icon_link('display',$url,$frog); }
function edit_link($url,$frog)    { return icon_link('edit'   ,$url,$frog); }
function pdf_link($url,$frog)     { return icon_link('pdf'    ,$url,$frog); }
function email_link($addr,$frog)  { return icon_link('email'  ,'mailto:' . $addr,$frog); }

function clone_popup($url,$frog)   { return icon_popup('clone'  ,$url,$frog); }
function delete_popup($url,$frog)  { return icon_popup('delete' ,$url,$frog); }
function display_popup($url,$frog) { return icon_popup('display',$url,$frog); }
function edit_popup($url,$frog)    { return icon_popup('edit'   ,$url,$frog); }
function pdf_popup($url,$frog)     { return icon_popup('pdf'    ,$url,$frog); }

function mailto_link($address,$frog) {
 global $cfg;

 $a = $address;
 if ($a && (strpos($a,'@') === false)) {
  $a .= '@' . $frog->cfg->default_email_domain;
 }

 if($a) {
  return(email_popup("mailto:{$a}",$frog));
 } else {
  return('');
 }
}

//////////////////////////////////////////////////////////////////////

/**
 * This returns HTML code for an icon with attached javascript that
 * can be clicked to toggle the visibility of another element. 
 *
 * @param int $id  The id attribute of the element to be toggled
 * @param bool $expanded  The initial state
 * @param string $type  The value of the display attribute for
 *                      the expanded state.
 *
 * If you want the toggled element to initially be visible, 
 * then you should set the second argument to 1.  If you want the
 * toggled element to initially be hidden, you can leave out
 * the second argument or set it to zero, but you also need
 * to set the CSS display attribute to 'none', for example with
 * markup like
 * <div id="hello" style="display:none">Hello World!</div>
 *
 * If the toggled element is a table row then the type argument
 * needs to be set to 'table-row'.  In almost all other cases
 * the empty string (implicitly converted by javascript to
 * 'block') is good.
 */

function toggler($id,$expanded = 0,$type='',$frog) {
 global $cfg;
 $f =  $expanded ? 'contract' : 'expand';
 $u = $frog->cfg->icons_url . '/' . $f . '.png';
 
 return <<<HTML
<img id="{$id}_toggler" alt="toggle" src="$u"
 onclick="{$frog->name}.toggle_display(this,'$id','$type')"/>
HTML;
}

//////////////////////////////////////////////////////////////////////

function checkbox($name,$value = 0,$opts=null) {
 $defaults = array(
  'onclick' => '',
  'onchange' => '',
  'disabled' => ''
 );
 
 foreach ($defaults as $k => $v) {
  $$k = $v;
  if ($opts && isset($opts[$k])) {
   $$k = $opts[$k];
   unset($opts[$k]);
  }
 }

 // Clicking a checkbox fires an onclick event but not an onchange 
 // event, so setting an onchange handler has no effect.  If the 
 // caller has asked to set an onchange handler, we set an onclick
 // handler instead.  This is convenient because it allows us to
 // treat different input elements more uniformly elsewhere in the
 // code.
 if ($onchange && ! $onclick) { $onclick = $onchange; }
 
 if ($onclick) {
  $oc = " onclick=\"$onclick\"";
 } else {
  $oc = '';
 }
 
 if ($disabled) {
  $dbd = " disabled=\"$disabled\"";
 } else {
  $dbd = '';
 }
 
 $c = $value ? ' checked="checked"' : '';
 return(<<<HTML
 <input type="checkbox" name="$name" $oc $dbd $c/>

HTML
);
}

//////////////////////////////////////////////////////////////////////

function forced_checkbox($name,$value,$frog) {
 $v = $value ? 1 : 0;
 $c = $v ? ' checked="checked"' : '';
 echo <<<HTML
<input type="hidden" name="$name" value="$v"/>
<input type="checkbox" name="{$name}_cb" onclick="{$frog->name}.force_checkbox()" $c/>
 
HTML;
}

//////////////////////////////////////////////////////////////////////

function selector($name,$values,$value='',$opts=null) {
 // generate a <select> ... </select> widget

 $defaults = array(
  'onchange' => '',
  'onfocus' => '',
  'onblur' => '',
  'width' => '',
  'mode' => 'simple',
  'index' => '',
  'display' => '',
  'empty_option' => false,
  'disabled' => '',
  'style' => ''
 );

 // mode can be simple, direct or indirect
 // * in simple mode, $values is just a list of strings 
 // * in direct mode, $values has entries like $k => $v,
 //   which give options with values $k displayed as $v
 // * in indirect mode, the entries in $values are objects $a,
 //   which give options with values $a->$index,  displayed
 //   as $a->$display
 foreach ($defaults as $k => $v) {
  $$k = $v;
  if ($opts && isset($opts[$k])) {
   $$k = $opts[$k];
  }
 }

 if ($onchange) {
  $oc = " onchange=\"$onchange\"";
 } else {
  $oc = '';
 }

 if ($onfocus) {
  $of = " onfocus=\"$onfocus\"";
 } else {
  $of = '';
 }

 if ($onblur) {
  $ob = " onblur=\"$onblur\"";
 } else {
  $ob = '';
 }

 if ($width) {
  $wd = " style=\"width: {$width}px\"";
 } else {
  $wd = '';
 }
 
 if ($disabled) {
  $dbd = " disabled=\"$disabled\"";
 } else {
  $dbd = '';
 }
 
 if ($style) {
  $style = " style=\"$style\"";
 } else {
  $style = '';
 }


 $html = <<<HTML
  <select name="$name"{$oc}{$of}{$ob}{$wd}{$dbd}{$style}>

HTML;

 if ($empty_option) {
  if ($value) {
   $html .= <<<HTML
  <option value="">&nbsp;</option>

HTML;
  } else {
   $html .= <<<HTML
  <option selected="selected" value="">&nbsp;</option>

HTML;

  }
 }

 $ss = ' selected="selected"';

 if ($mode == 'simple') {
  foreach ($values as $v) {
   $s = ($v == $value) ? $ss : '';
   $html .= "<option$s>$v</option>\r\n";
  }
 } elseif ($mode == 'direct') {
  foreach ($values as $k => $v) {
   $s = ($k == $value) ? $ss : '';
   $html .= "<option value=\"$k\"$s>$v</option>\r\n";
  }
 } elseif($mode == 'indirect') {
  foreach ($values as $a) {
   $k = $a->$index;
   $v = $a->$display;
   $s = ($k == $value) ? $ss : '';
   $html .= "<option value=\"$k\"$s>$v</option>\r\n";
  }
 }

 $html .= <<<HTML
 </select>

HTML;

 return($html);
}

/////////////////////////////////////////////

function selector_td($name,$values,$value='',$opts=null) {
 $html = "<td>\n";
 $html .= selector($name,$values,$value,$opts);
 $html .= "</td>";
 
 return($html);
}

//////////////////////////////////////////////////////////////////////

function session_selector($field_name,$session = null,$opts=null) {
 $y = date('Y');
 
 $defaults = array(
  'onchange' => null,
  'empty_option' => true,
  'width' => '',
  'min_session' => min($session-1,$y),
  'max_session' => max($session+1,$y+3)
 );

 foreach ($defaults as $k => $v) {
  $$k = $v;
  if ($opts && isset($opts[$k])) {
   $$k = $opts[$k];
  }
 }

 $oc = '';
 if ($onchange) { $oc = " onchange=\"$onchange\""; }

 $eo = "\r\n <option value=\"\">&nbsp;</option>";

 if ($width) {
  $wd = " style=\"width: {$width}px\"";
 } else {
  $wd = '';
 }

 $html = <<<HTML
<select name="$field_name"$oc$wd>$eo

HTML;
 for ($i = $min_session; $i <= $max_session; $i++ ){
  $s = ($session == $i) ? ' selected="selected"' : '';
  $ii = sprintf("%d/%02d", $i, $i-1999);
  $html .= <<<HTML
 <option value="$i"$s>$ii</option>
 
HTML;
 }

 $html .= <<<HTML
</select>

HTML;

 return($html);
}

//////////////////////////////////////////////////////////////////////

function semester_selector($field_name,$semester = null,$opts=null) {
 $defaults = array(
  'onchange' => null,
  'width' => null,
  'empty_option' => true,
  'allow_resit' => false,
  'allow_year' => false
 );

 foreach ($defaults as $k => $v) {
  $$k = $v;
  if ($opts && isset($opts[$k])) {
   $$k = $opts[$k];
  }
 }

 $oc = '';
 if ($onchange) { $oc = " onchange=\"$onchange\""; }

 $wd = '';
 if ($width) { $wd = " style=\"width: {$width}px\""; }

 $eo = ''; $ro = ''; $yo = '';

 $eo = "\r\n <option value=\"\">&nbsp;</option>";
 $s1 = ($semester == '1') ? ' selected' : '';
 $s2 = ($semester == '2') ? ' selected' : '';

 if ($allow_resit) {
  $sr = ($semester == 'resit') ? ' selected' : '';
  $ro = "\r\n <option $sr>resit</option>";
 }

 if ($allow_year) {
  $sy = ($semester == 'year') ? ' selected' : '';
  $yo = "\r\n <option $sr>year</option>";
 }

 return( <<<HTML
<select name="$field_name"$oc$wd>$eo
 <option$s1>1</option>
 <option$s2>2</option>$ro$yo
</select>

HTML
	 );
}

function full_semester_selector($field_name,$semester) {
 return selector($field_name,
		 array('1' => '1 (Autumn)',
		       '2' => '2 (Spring)',
		       'Year' => 'Full year'),
		 $semester,
		 array('mode' => 'direct')
		 );
}

//////////////////////////////////////////////////////////////////////

function ajax_selector($type,$field_name,$id='',$val='',$opts=null) {
 $oo = array(
  'size'          => '30',
  'style'         => '',
  'onchange'      => '',
  'extra'         => array()
 );

 if ($opts) {
  foreach ($opts as $k => $v) { $oo[$k] = $v; }
 }

 $oc = $oo['onchange'] ? ' onchange="' . $oo['onchange'] . '"' : '';
 $st = $oo['style']    ? ' style="' . $oo['style'] . '"' : '';
 $sz = $oo['size']     ? ' size="' . $oo['size'] . '"' : '';
 $v = htmlentities($val);

 $ii = '';
 if ($id) { $ii = ",$id"; }

 $extra = '';
 foreach ($oo['extra'] as $k => $v) {
  $extra .= " $k=\"$v\"";
 }

 return <<<HTML
<input type="text" name="{$field_name}_display" id="{$field_name}_display" class="autosuggest" $sz $st value="$v"/>
<input type="hidden" name="$field_name" id="$field_name" class="autosuggest_{$type}" value="$id" $oc $extra/>

HTML;
}

//////////////////////////////////////////////////////////////////////

function date_box($id,$date,$frog,$opts = null) {
 $onchange = ($opts && isset($opts['onchange'])) ? $opts['onchange'] : '';
 $opts['onchange'] = NULL;

 $pm = '';
 foreach ($opts as $k => $v) {
  if ($v) {
   $pm .= " $k=\"$v\"";
  }
 }

 return <<<HTML
 <input type="text" class="date"
  name="$id" id="$id"
  onchange="{$frog->name}.check_date('$id');$onchange"
  value="$date" $pm
 /><img class="link" alt="calendar" id="{$id}_button" src="{$frog->cfg->icons_url}/calendar.png"/>

HTML;
}

//////////////////////////////////////////////////////////////////////

function latex_edit_pane($m,$f,$opts = null) {
 $lf = strtolower($f);
 $fh = "{$lf}_html";
 $x = htmlentities($m->$fh);

 $defaults = array(
  'latex_top'     => '',
  'latex_bottom'  => '',
  'instructions'  => '',
  'tab_title'     => $f,
  'choices'       => null,
  'choices_label' => 'Standard choices'
 );

 foreach ($defaults as $k => $v) {
  $$k = $v;
  if ($opts && isset($opts[$k])) {
   $$k = $opts[$k];
  }
 }

 $lt = htmlentities($latex_top);
 $lb = htmlentities($latex_bottom);
 $mlf = trim($m->$lf);
 $mfh = trim($m->$fh);

 // Strip out empty lists to pacify the HTML validator
 $mfh = preg_replace('!<ul></ul>!','<br/>',$mfh);

 $instructions = trim($instructions);

 if ($instructions) {
  $instructions = '<div class="text">' . $instructions . '</div>';
 }

 echo <<<HTML
<div class="tabbertab">
<h2>$tab_title</h2>
<p>This may include LaTeX.
 &nbsp; &nbsp; &nbsp;
 <a href="javascript:{$frog->name}.convert_latex('$lf')">Convert to HTML</a>
</p>

<div class="text">$instructions</div>

<input type="hidden" name="{$lf}_latex_top" value="$lt"/>
<textarea name="$lf" rows="15" cols="100"
  onblur="frog.convert_latex('$lf');">
$mlf
</textarea>
<br/>

HTML;

 if ($choices) {
  echo <<<HTML
 &nbsp; &nbsp; &nbsp; $choices_label
 <select name="{$lf}_choice" 
  onChange="document.main_form.{$lf}.value=document.main_form.{$lf}_choice.value;frog.convert_latex('$lf');"
 >

HTML;
  foreach ($choices as $c) {
   $s = ($c == $mlf) ? " selected=\"selected\"" : '';
   if ($c) {
    $v = '';
    $d = $c;
   } else {
    $v = ' value=""';
    $d = '&nbsp;';
   }
   echo <<<HTML
  <option$v$s>$d</option>

HTML;
  }
  echo <<<HTML
 </select>

HTML;
 }

 echo <<<HTML
<input type="hidden" name="{$lf}_latex_bottom" value="$lb"/>

<input type="hidden" name="$fh" value="$x"/>
<br/>
<br/>
<b>HTML Version</b>
<br/>
<br/>

<div id="div_{$fh}" style="width: 700px">
$mfh
</div>

<br/>
</div>

HTML;
}

//////////////////////////////////////////////////////////////////////

function mathjax_edit_pane($m,$f,$opts = null) {
 $lf = strtolower($f);
 $fh = "{$lf}_html";
 $x = htmlentities($m->$fh);

 $defaults = array(
  'instructions'  => '',
  'tab_title'     => $f,
  'choices'       => null,
  'choices_label' => 'Standard choices'
 );

 foreach ($defaults as $k => $v) {
  $$k = $v;
  if ($opts && isset($opts[$k])) {
   $$k = $opts[$k];
  }
 }

 $mlf = trim($m->$lf);
 $mfh = latex_to_mathjax($mlf);

 // Strip out empty lists to pacify the HTML validator
 $mfh = preg_replace('!<ul></ul>!','<br/>',$mfh);

 $instructions = trim($instructions);

 if ($instructions) {
  $instructions = '<div class="text">' . $instructions . '</div>';
 }

 echo <<<HTML
<div class="tabbertab">
<h2>$tab_title</h2>
<p>This may include LaTeX.
 &nbsp; &nbsp; &nbsp;
 <a href="javascript:frog.convert_mathjax('$lf')">Convert to HTML</a>
</p>

<div class="text">$instructions</div>

<textarea name="$lf" rows="15" cols="100"
  onblur="frog.convert_mathjax('$lf');">
$mlf
</textarea>
<br/>

HTML;

 if ($choices) {
  echo <<<HTML
 &nbsp; &nbsp; &nbsp; $choices_label
 <select name="{$lf}_choice" 
  onChange="document.main_form.{$lf}.value=document.main_form.{$lf}_choice.value;frog.convert_mathjax('$lf');"
 >

HTML;
  foreach ($choices as $c) {
   $s = ($c == $mlf) ? " selected=\"selected\"" : '';
   if ($c) {
    $v = '';
    $d = $c;
   } else {
    $v = ' value=""';
    $d = '&nbsp;';
   }
   echo <<<HTML
  <option$v$s>$d</option>

HTML;
  }
  echo <<<HTML
 </select>

HTML;
 }

 echo <<<HTML
<input type="hidden" name="$fh" value="$x"/>
<br/>
<br/>
<b>HTML Version</b>
<br/>
<br/>

<div id="div_{$fh}" style="width: 700px">
$mfh
</div>

<br/>
</div>

HTML;
}

//////////////////////////////////////////////////////////////////////

function text_edit_pane($m,$f,$opts = null) {
 $lf = strtolower($f);

 $defaults = array(
  'instructions'  => '',
  'tab_title'     => $f,
  'choices'       => null,
  'choices_label' => 'Standard choices'
 );

 foreach ($defaults as $k => $v) {
  $$k = $v;
  if ($opts && isset($opts[$k])) {
   $$k = $opts[$k];
  }
 }

 $mf = trim($m->$lf);
 $mfe = htmlentities($mf);

 // Strip out empty lists to pacify the HTML validator
 $mfh = preg_replace('!<ul></ul>!','<br/>',$mf);
 echo <<<HTML
<div class="tabbertab">
<h2>$tab_title</h2>

$instructions<br/>

<textarea id="$lf" name="$lf" rows="15" cols="100" onchange="frog.text_pane_onchange('$lf')">
$mfe
</textarea>
<br/>

HTML;

 if ($choices) {
  echo <<<HTML
 &nbsp; &nbsp; &nbsp; $choices_label
 <select id="{$lf}_choice" name="{$lf}_choice" 
  onchange="frog.text_pane_select_choice('$lf')">

HTML;
  foreach ($choices as $c) {
   $s = ($c == $mlf) ? " selected=\"selected\"" : '';
   echo <<<HTML
  <option$s>$c</option>

HTML;
  }
  echo <<<HTML
 </select>

HTML;
 }

 echo <<<HTML

<br/>
<table><tr><td class="command" onclick="frog.text_pane_preview('$lf')">Preview</td></tr></table>
<br/>
<b>Formatted version</b>
<br/>
<br/>

<div id="div_{$lf}" style="width: 700px">$mf</div>

<br/>
</div>

HTML;
}

//////////////////////////////////////////////////////////////////////

function debug_pane() {
  echo <<<HTML
<div class="tabbertab">
<h2>Debug</h2>
<br/>

HTML;

  var_dump($_REQUEST);

echo <<<HTML
</div>

HTML;
}

//////////////////////////////////////////////////////////////////////

function hidden_input($field_name,$value) {
 $v = htmlentities($value);
 return <<<HTML
<input type="hidden" name="$field_name" value="$v"/>

HTML;
}

//////////////////////////////////////////////////////////////////////

function text_input($field_name,$value,$opts = null) {
 
  $pm = '';
  
  if ($opts) {
	foreach ($opts as $k => $v) {
		$pm .= " $k=\"$v\"";
	}
  }
  
  $v = htmlentities($value);
 return <<<HTML
<input type="text" name="$field_name" value="$v" {$pm}/>

HTML;
}

//////////////////////////////////////////////////////////////////////

function file_input($field_name,$opts = null) {
 $sz = '';
 $oc = '';

 if ($opts) {
  if (isset($opts['size'])) { $sz = ' size="' . $opts['size'] . '"'; } 
  if (isset($opts['onchange'])) { $oc = ' onchange="' . $opts['onchange'] . '"'; }
 }

 return <<<HTML
<input type="file" name="$field_name" {$sz}{$oc}/>

HTML;
}

//////////////////////////////////////////////////////////////////////

function url_input($field_name,$value,$opts = null) {
 $sz = '';
 $oc = '';

 if ($opts) {
  if (isset($opts['size'])) { $sz = ' size="' . $opts['size'] . '"'; } 
  if (isset($opts['onchange'])) { $oc = ' onchange="' . $opts['onchange'] . '"'; }
 }

 $v = htmlentities($value);
 return <<<HTML
<input type="text" name="$field_name" id="$field_name" value="$v"{$sz}{$oc}/>
<a id="{$field_name}_check_button"
 href="javascript:frog.check_url_element('$field_name')">Check</a>

HTML;
}

//////////////////////////////////////////////////////////////////////

function textarea($field_name,$value,$opts = null) {
 $opts['rows'] = isset($opts['rows']) ? $opts['rows'] : 5;
 $opts['cols'] = isset($opts['cols']) ? $opts['cols'] : 50;
 
 $pm = '';
 foreach ($opts as $k => $v) {
  $pm .= " $k = \"$v\"";
  }

 $val = htmlentities($value);
 return <<<HTML
<textarea name="$field_name" {$pm}>$val</textarea>

HTML;
}

//////////////////////////////////////////////////////////////////////

function command_table_html($commands) {
 $h = <<<HTML
<table>
 <tr>

HTML;

 foreach($commands as $k => $v) {
  $h .= <<<HTML
  <td class="command" onclick="$v">$k</td>

HTML;
 }

 $h .= <<<HTML
 </tr>
</table>

HTML;

 return($h);
}

function command_table($commands) {
 echo command_table_html($commands);
}

/////////////////////////////////////////////////////////////////////

function asterisk() {
 return '<font color="red">*</font>&nbsp;';
}

//////////////////////////////////////////////////

function itemize_to_ul($string) {
	$s = str_replace("\begin{itemize}","<ul>",$string);
	$s = str_replace("\end{itemize}","</ul>",$s);
	$s = str_replace("\item","<li>",$s);
	$s = nl2br($s);
	return $s;
}


?>