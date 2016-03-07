setCookie = function(cname, cvalue, exdays) {
    var d = new Date();
    d.setTime(d.getTime() + (exdays * 24 * 60 * 60 * 1000));
    var expires = "expires=" + d.toUTCString();
    document.cookie = cname + "=" + cvalue + "; " + expires;
}

getCookie = function(cname) {
    var name = cname + "=";
    var ca = document.cookie.split(';');
    for(var i=0; i < ca.length; i++) {
        var c = ca[i];
        while (c.charAt(0) == ' ') c = c.substring(1);
        if (c.indexOf(name) == 0) return c.substring(name.length, c.length);
    }
    return "";
}

ord = function(i){
  return String.fromCharCode(48 + i);
}

getNum = function(s, i, m){
  var n = s.charCodeAt(i)-176;
  if(n > m){n = m;}  
  return ord(n);
}

//get gamestate from yucata string
getGameState = function(){
  var s = gameState.getStatusString();
  var n = "";
  var got = Array();
  got[0] = Array();
  got[0][1] = getNum(s, 37, 2);
  got[0][2] = getNum(s, 40, 3);
  got[0][3] = getNum(s, 43, 4);
  got[0][4] = getNum(s, 46, 5);
  got[0][5] = getNum(s, 49, 6);
  got[0][6] = getNum(s, 52, 6);
  got[1] = Array();
  got[1][1]=getNum(s, 61, 2);
  got[1][2]=getNum(s, 64, 3);
  got[1][3]=getNum(s, 67, 4);
  got[1][4]=getNum(s, 70, 5);
  got[1][5]=getNum(s, 73, 6);
  got[1][6]=getNum(s, 76, 6);
  
  if(s.charCodeAt(55) - 176 != 0){
    got[0][s.charCodeAt(55) - 176]--;
    if(got[0][s.charCodeAt(55) - 176] < 0) got[0][s.charCodeAt(55) - 176] = 0;
  }
  if(s.charCodeAt(57) - 176 != 0){
    got[0][s.charCodeAt(57) - 176]--;
    if(got[0][s.charCodeAt(57) - 176] < 0) got[0][s.charCodeAt(57) - 176] = 0;
  }
  if(s.charCodeAt(59) - 176 != 0){
    got[0][s.charCodeAt(59) - 176]--;
    if(got[0][s.charCodeAt(59) - 176] < 0) got[0][s.charCodeAt(59) - 176] = 0;
  }

  if(s.charCodeAt(79) - 176 != 0){
    got[1][s.charCodeAt(79) - 176]--;
    if(got[1][s.charCodeAt(79) - 176] < 0) got[1][s.charCodeAt(79) - 176] = 0;
  }
  if(s.charCodeAt(81) - 176 != 0){
    got[1][s.charCodeAt(81) - 176]--;
    if(got[1][s.charCodeAt(81) - 176] < 0) got[1][s.charCodeAt(81) - 176] = 0;
  }
  if(s.charCodeAt(83) - 176 != 0){
    got[1][s.charCodeAt(83) - 176]--;
    if(got[1][s.charCodeAt(83) - 176] < 0) got[1][s.charCodeAt(83) - 176] = 0;
  }
  
  for(var i = 0; i < 2; i++){
    for(var j = 1; j <= 6; j++){
      n += got[i][j];
    }
  }
  var first = Array('-','-','-','-','-');
  if(s.charCodeAt(39) == 49){ first[0]=0; }
  if(s.charCodeAt(42) == 49){ first[1]=0; }
  if(s.charCodeAt(45) == 49){ first[2]=0; }
  if(s.charCodeAt(48) == 49){ first[3]=0; }
  if(s.charCodeAt(51) == 49){ first[4]=0; }
  
  if(s.charCodeAt(63) == 49){ first[0]=1; }
  if(s.charCodeAt(66) == 49){ first[1]=1; }
  if(s.charCodeAt(69) == 49){ first[2]=1; }
  if(s.charCodeAt(72) == 49){ first[3]=1; }
  if(s.charCodeAt(75) == 49){ first[4]=1; }
  
  return n + first[0] + first[1] + first[2] + first[3] + first[4];
}

//get roundstate from yucata string
getRoundState = function(){
  var s = gameState.getStatusString();
  var ret = "";
  //dices remaining
  ret += s.charCodeAt(9) - 176;
  //rolled
  ret += s.charCodeAt(10) - 176;
  ret += s.charCodeAt(11) - 176;
  //trucks
  ret += s.charCodeAt(12) - 176;
  ret += s.charCodeAt(14) - 176;
  ret += s.charCodeAt(16) - 176;
  ret += s.charCodeAt(18) - 176;
  ret += s.charCodeAt(20) - 176;
  ret += s.charCodeAt(22) - 176;
  ret += s.charCodeAt(24) - 176;
  ret += s.charCodeAt(26) - 176;
  ret += s.charCodeAt(28) - 176;
  //taken
  ret += s.charCodeAt(55) - 176;
  ret += s.charCodeAt(57) - 176;
  ret += s.charCodeAt(59) - 176;
  ret += s.charCodeAt(79) - 176;
  ret += s.charCodeAt(81) - 176;
  ret += s.charCodeAt(83) - 176;
  return ret;
}

swapRound = function(state){
  var ret = "";
  for(var i=0;i<12;i++){
    ret += state[i];
  }
  for(var i=0;i<3;i++){
    ret += state[12+3+i];
  }
  for(var i=0;i<3;i++){
    ret += state[12+i];
  }
  return ret;
}

swapGame = function(state){
  var res = "";
  for(var i=0;i<6;i++){
    res += state[6+i];
  }
  for(var i=0;i<6;i++){
    res += state[i];
  }

  for(var i=12;i<12+5;i++){
    if(state[i]=='0'){
      res += "1";
    }else if(state[i]=='1'){
      res += "0";
    }else{
      res += "-";
    }
  }
  return res;
}

showCommand = function(com){
  $('.command').remove();
  if(com=="r"){
    $("<div class='command'><img src='http://atrey.karlin.mff.cuni.cz/~savlik/zoo/circle.gif' style='width:70px;height:90px' /></div>").css({
      position: "absolute",
      top: 130,
      left: 5,
      width: "0px",
      height: "0px",
      "pointer-events": "none",
      "z-index": 1000
    }).appendTo($("body"));
  }else if(com.length==1){
    var offset = (com-1)*95;
    $("<div class='command'><img src='http://atrey.karlin.mff.cuni.cz/~savlik/zoo/circle.gif' style='width:250px;height:110px' /></div>").css({
      position: "absolute",
      top: 240+offset,
      left: 5,
      width: "0px",
      height: "0px",
      "pointer-events": "none",
      "z-index": 1000
    }).appendTo($("body"));
  }else if(com.length==2){
    //first arrow
    var offset;
    offset = (com[0]-1) * 95;
    $("<div class='command'><img src='http://atrey.karlin.mff.cuni.cz/~savlik/zoo/arrow.png' style='width:30px;height:"+(120+offset)+"px' /></div>").css({
      position: "absolute",
      top: 175,
      left: 84,
      width: "0px",
      height: "0px",
      "pointer-events": "none",
      "z-index": 1000
    }).appendTo($("body"));
    offset = (com[1]-1) * 95;
    $("<div class='command'><img src='http://atrey.karlin.mff.cuni.cz/~savlik/zoo/arrow.png' style='width:30px;height:"+(120+offset)+"px' /></div>").css({
      position: "absolute",
      top: 175,
      left: 134,
      width: "0px",
      height: "0px",
      "pointer-events": "none",
      "z-index": 1000
    }).appendTo($("body"));
  }
  
}

StarterIndex = "";
init = function(){
  StarterIndex = (gameState.getStatusString().charCodeAt(8)-176);
  
  $("<div id='bar'></div>").css({
    position: "absolute",
    top: 50,
    left: 270,
    width: "450px",
    height: "100px",
    background: "silver",
    border: "1px solid black",
    padding: "10px",
    "z-index": 1000
  }).appendTo($("body"));
  
  $("<div id='per'></div>").css({
    "width": "200px",
    "height": "70px",
    margin: "15px",
    float: "left",
    "font-size": "50px",
    "font-weight": 550,
    "text-shadow": "-1px -1px 0 #000, 1px -1px 0 #000, -1px 1px 0 #000, 1px 1px 0 #000"
  }).appendTo($("#bar"));

  $("<div id='col_0'></div>").css({
    "width": "100px",
    "height": "90px",
    margin: "5px",
    float: "left",
    "font-size": "13px"
  }).appendTo($("#bar"));

  $("<div id='col_1'></div>").css({
    "width": "100px",
    "height": "90px",
    margin: "5px",
    float: "left",
    "font-size": "13px"
  }).appendTo($("#bar"));
  
  $("<div id='toggle'>hide</div>").css({
    position: "absolute",
    top: "3px",
    right: "5px",
    cursor: "pointer",
    "font-size": "10px"
  }).appendTo($("#bar")).click(function(){toggle()});
}

hidden = function(){
  if(getCookie("hidden")=="1") return true;
  return false;
}

toggle = function(){
  if(hidden()){  //show
    $('#toggle').text("hide");
    setCookie("hidden","0",365);
    $('#bar').css("height","100px");
    $('#per').css("visibility",""); 
    $('#col_0').css("visibility","");
    $('#col_1').css("visibility","");
    $('.command').css("visibility","");
    
  }else{  //hide
    $('#toggle').text("show");
    setCookie("hidden","1",365);
    $('#bar').css("height","0px");
    $('#per').css("visibility","hidden"); 
    $('#col_0').css("visibility","hidden");
    $('#col_1').css("visibility","hidden");
    $('.command').css("visibility","hidden");
    
  }
  
}

update = function(){
  var myGameState = getGameState();
  var roundState = getRoundState();
  if(StarterIndex==1){
    myGameState = swapGame(myGameState);
    roundState = swapRound(roundState);
  }

  var myTurn = (UserIndex==OnTurnIndex);
  if(myTurn){
    console.log("my turn");
  }else{
    console.log("his turn");
  }
  console.log(myGameState);
  console.log(roundState);

  $('#col_0').html("");
  $('#col_1').html("");
  changePercent("");
  showCommand("");  
  
  $("#per").html("<img src='http://atrey.karlin.mff.cuni.cz/~savlik/zoo/loading.gif' />");
  toggle();toggle();
  
  var result = Array();
  $.ajax({
    url: "http://atrey.karlin.mff.cuni.cz/~savlik/zoo/helper.php", 
    success: function(data){
      result = JSON.parse(data);
      
      
      for(var i = 0; i<result.length; i++){
        if(StarterIndex != UserIndex){
          result[i][1] = 1 -  parseFloat(result[i][1]);
        }else{
          result[i][1] =  parseFloat(result[i][1]);
        }
      }
           
      if(result.length!=0){
        var best = myTurn?-1:2;
        var besti = -1;
        
        for(var i = 0; i<result.length; i++){
          
          if(myTurn && result[i][1]>best){
            best = result[i][1];
            besti = i;
          }
          
          if(!myTurn && result[i][1]<best){
            best = result[i][1];
            besti = i;
          }
        }

        for(var i = 0; i<result.length; i++){
          var added = result[i][0] + ': ' + round(result[i][1]) + "%<br />";
          if(i==besti){
            added = "<b>"+added+"</b>";
          }
          console.log(added);
          $(i<5?'#col_0':'#col_1').html($(i<5?'#col_0':'#col_1').html() + added);
        }
        changePercent(best);
        showCommand(result[besti][0]);
        toggle();toggle();
      }

    },
    type: "GET",
    data: { gs : myGameState, rs : roundState, name : UserLogin }
  });
}

decimalToHex = function(d, padding) {
  var hex = Number(d).toString(16);
  padding = typeof (padding) === "undefined" || padding === null ? padding = 2 : padding;
  while (hex.length < padding) {
    hex = "0" + hex;
  }
  return hex;
}

round = function(per){
  return Math.round(per*10000)/100;
}

changePercent = function(per){
  if(per.length==0){$("#per").text(""); return;}
  var str = "#";
  var red = Math.min(255, Math.floor(255*(1-per)*2));
  var green = Math.min(255, Math.floor(255*per*2));
  var blue = 0;
  str+= decimalToHex(red,2) + decimalToHex(green,2) + decimalToHex(blue,2);
  $("#per").text(round(per) + "%").css({"color":str});
}


//addons
game.oldRollTwoDice = game.RollTwoDice ;
game.RollTwoDice = function(){
  var result = game.oldRollTwoDice.apply(this, arguments);
  //setTimeout("window.location = window.location.href;", 750);	
  update();
  console.log("RollTwoDice");
  return result ;
}

$("input").css({"padding-top":"10px", "padding-bottom":"10px"});

init();
update();

