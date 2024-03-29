<?php

$latex_to_mathjax_search = array(
 '\%',
 '\newline',
 '\medskip',
 '\bigskip',
 '\[',
 '\]',
 '\N',
 '\Z',
 '\Q',
 '\R',
 '\C',
 '\begin{center}',
 '\end{center}',
 '\begin{quote}',
 '\end{quote}',
 '\begin{itemize}',
 '\end{itemize}',
 '\begin{enumerate}',
 '\end{enumerate}',
 '\item',
 '\`A',
 '\\\'a',
 '\^A',
 '\"A',
 '\`E',
 '\\\'E',
 '\^E',
 '\"E',
 '\`I',
 '\\\'I',
 '\^I',
 '\"I',
 '\`O',
 '\\\'O',
 '\^O',
 '\"O',
 '\`U',
 '\\\'U',
 '\^U',
 '\"U',
 '\`a',
 '\\\'a',
 '\^a',
 '\"a',
 '\`e',
 '\\\'e',
 '\^e',
 '\"e',
 '\`i',
 '\\\'i',
 '\^i',
 '\"i',
 '\~n',
 '\\\'n',
 '\`o',
 '\\\'o',
 '\^o',
 '\~o',
 '\"o',
 '\`u',
 '\\\'u',
 '\^u',
 '\"u',
 '\`{A}',
 '\\\'{a}',
 '\^{A}',
 '\"{A}',
 '\`{E}',
 '\\\'{E}',
 '\^{E}',
 '\"{E}',
 '\`{I}',
 '\\\'{I}',
 '\^{I}',
 '\"{I}',
 '\`{O}',
 '\\\'{O}',
 '\^{O}',
 '\"{O}',
 '\`{U}',
 '\\\'{U}',
 '\^{U}',
 '\"{U}',
 '\`{a}',
 '\\\'{a}',
 '\^{a}',
 '\"{a}',
 '\`{e}',
 '\\\'{e}',
 '\^{e}',
 '\"{e}',
 '\`{i}',
 '\\\'{i}',
 '\^{i}',
 '\"{i}',
 '\~{n}',
 '\\\'{n}',
 '\`{o}',
 '\\\'{o}',
 '\^{o}',
 '\~{o}',
 '\"{o}',
 '\`{u}',
 '\\\'{u}',
 '\^{u}',
 '\"{u}',
);

$latex_to_mathjax_replace = array(
 '%',
 '<br/>',
 '<br/><br/>',
 '<br/><br/><br/>',
 '$$',
 '$$',
 '\mathbb{N}',
 '\mathbb{Z}',
 '\mathbb{Q}',
 '\mathbb{R}',
 '\mathbb{C}',
 '<center>',
 '</center>',
 '<blockquote>',
 '</blockquote>',
 '<ul>',
 '</ul>',
 '<ol>',
 '</ol>',
 '<li>',
 'À',
 'Á',
 'Â',
 'Ä',
 'È',
 'É',
 'Ê',
 'Ë',
 'Ì',
 'Í',
 'Î',
 'Ï',
 'Ò',
 'Ó',
 'Ô',
 'Ö',
 'Ù',
 'Ú',
 'Û',
 'Ü',
 'à',
 'á',
 'â',
 'ä',
 'è',
 'é',
 'ê',
 'ë',
 'ì',
 'í',
 'î',
 'ï',
 'ñ',
 'ń',
 'ò',
 'ó',
 'ô',
 'õ',
 'ö',
 'ù',
 'ú',
 'û',
 'ü',
 'À',
 'Á',
 'Â',
 'Ä',
 'È',
 'É',
 'Ê',
 'Ë',
 'Ì',
 'Í',
 'Î',
 'Ï',
 'Ò',
 'Ó',
 'Ô',
 'Ö',
 'Ù',
 'Ú',
 'Û',
 'Ü',
 'à',
 'á',
 'â',
 'ä',
 'è',
 'é',
 'ê',
 'ë',
 'ì',
 'í',
 'î',
 'ï',
 'ñ',
 'ń',
 'ò',
 'ó',
 'ô',
 'õ',
 'ö',
 'ù',
 'ú',
 'û',
 'ü',
);

function latex_to_mathjax($s) {
 global $latex_to_mathjax_search,$latex_to_mathjax_replace;

 $t = $s;

 $t = preg_replace('/[^\\\\]\\\\ /',' ',$t);
 $t = preg_replace('/(\\n%([^\\n]*))*/','',$t);

 $t = str_replace($latex_to_mathjax_search,$latex_to_mathjax_replace,$t);

 $t = preg_replace('/{\\\\em ([^{}]*)}/','<em>$1</em>',$t);
 $t = preg_replace('/\\\\emph{([^{}]*)}/','<em>$1</em>',$t);

 $t = preg_replace('/{\\\\it ([^{}]*)}/','<em>$1</em>',$t);
 $t = preg_replace('/\\\\textit{([^{}]*)}/','<em>$1</em>',$t);

 $t = preg_replace('/{\\\\bf ([^{}]*)}/','<b>$1</b>',$t);
 $t = preg_replace('/\\\\textbf{([^{}]*)}/','<b>$1</b>',$t);

 $t = preg_replace('/{\\\\tt ([^{}]*)}/','<tt>$1</tt>',$t);
 $t = preg_replace('/\\\\texttt{([^{}]*)}/','<tt>$1</tt>',$t);

 $t = preg_replace('/\\\\cite{([^{}]*)}/','[<span class="cite">$1</span>]',$t);
 $t = preg_replace('/\\\\url{([^{}]*)}/','<a href="$1">$1</a>',$t);

 return($t);
}

?>
