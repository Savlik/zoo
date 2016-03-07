<?php
header("Access-Control-Allow-Origin: *");

$gs = $_GET['gs'];
$rs = $_GET['rs'];
$name = $_GET['name'];
$file = fopen("log.log","a");

fputs($file, $_SERVER[ "REMOTE_ADDR" ]." | ".date( "Y-m-d,H:i:sO" )." | ".$name."\n");

if(!preg_match("/^[0123456789-]+$/", $gs.$rs)){
  echo "[\"\"]";
}else{
  passthru ("./zoo.o help ".$gs." ".$rs);
}

?>
