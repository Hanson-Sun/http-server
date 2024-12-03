var myRange = document.getElementById('myRange');
var output = document.getElementById('output');
var appleArea = document.getElementById('appleArea');
var countdownInterval;
var currentCount;
var goodAppleCount = 0;
var goodAppleMax = 0;
var NewgoodAppleMax = 0;
var greenApple = '<img src="images/badapple1.png" class="bad1" onmouseover="chop(this.id)" ';
var redApple = '<img src="images/goodapple1.png" class="good1" onmouseover="chop(this.id)"';
var badapple2 = '<img src="images/badapple2.png" class="bad2" onmouseover="chop(this.id)"';
var goodapple2 = '<img src="images/goodapple2.png" class="good2" onmouseover="chop(this.id)"';
var bgAudio = document.getElementById("bgAudio");

output.innerHTML = myRange.value;
document.getElementById('timebar').style.display = 'none';
document.getElementById('restartBtn').style.display = 'none';

function updateOutput() {
    output.innerHTML = myRange.value;
}


function genApple() {
    bgAudio.play();
    bgAudio.loop = true;
    bgAudio.volume = 0.5;
    var howManyApples = myRange.value;
    var goodId, badId;
    for (i = 0; i < howManyApples; i++) {
        goodId = 'id="good' + i + '">';
        badId = 'id="bad' + i + '">';
        if (Math.random() < 0.8) {
            appleArea.innerHTML += redApple + goodId;
            goodAppleMax++;
        } else {
            appleArea.innerHTML += greenApple + badId;
        }
    }
    document.getElementById('genBtn').style.display = 'none';
    document.getElementById('restartBtn').style.display = '';
    document.getElementById('timebar').style.display = '';
    timeAllowed = Math.round(howManyApples / 2) + 1;
    countdown(howManyApples);
    doCount();

}

function restart() {
    bgAudio.load();
    NewgoodAppleMax = 0;
    goodAppleMax = 0;
    goodAppleCount = 0;
    howManyApples = myRange.value;
    appleArea.innerHTML = '';
    document.getElementById('genBtn').style.display = '';
    document.getElementById('restartBtn').style.display = 'none';
    goodcount = 0;
    clearInterval(countdownInterval);
    timeAllowed = Math.round(howManyApples / 2) + 1;
    currentCount = timeAllowed;
    document.getElementById('timebar').style.display = "none";
}


function chop(appleId) {
    chopsound();
    var isGood = appleId[0] == 'g';
    var isBad = appleId[0] == 'b';

    console.log(goodAppleCount, goodAppleMax);

    if ((document.getElementById(appleId).className) == 'good1') {
        goodAppleCount++;
    }

    if (isBad) {
        document.getElementById(appleId).className = 'bad2';
        currentCount = currentCount - 1;
        scream();
        document.getElementById("play").style.backgroundColor = "rgb(218, 32, 32)";
        setTimeout(function () {
            var goodId, badId;
            for (i = 0; i < 3; i++) {
                goodId = 'id="good' + (goodAppleMax + 50 + i) + '">';
                badId = 'id="bad' + (goodAppleMax + 20 + i) + '">';
                if (Math.random() < 0.8) {
                    appleArea.innerHTML += redApple + goodId;
                    NewgoodAppleMax++;
                    console.log(goodId);
                } else {
                    appleArea.innerHTML += greenApple + badId;
                }
            }
        }, 300);
        goodAppleMax = goodAppleMax + NewgoodAppleMax;
        
    }
    if (isGood) {
        document.getElementById(appleId).className = 'good2';
        document.getElementById("play").style.backgroundColor = "rgb(50, 205, 202)";

    }
    if (goodAppleCount == (goodAppleMax+NewgoodAppleMax)) {
        document.getElementById("appleArea").innerHTML = "<h1>You WIN!</h1>";
        clearInterval(countdownInterval);
        bgAudio.load();
        win();
    }
}
var timeAllowed = Math.round(howManyApples / 2) + 1;
function countdown(howManyApples) {
    
    document.getElementById("countdown").innerHTML = timeAllowed;
    currentCount = timeAllowed;
    clearInterval(countdownInterval);
    countdownInterval = setInterval(doCount, 100);
}
function doCount() {

    currentCount = (currentCount - 0.1).toFixed(1);
    document.getElementById('countdown').innerHTML = currentCount * 10;
    
    if (currentCount * 1 <= 0) {
        // time is up - you lose
        document.getElementById("appleArea").innerHTML = "<h1>You lose, your time ran out.</h1>";
        clearInterval(countdownInterval);
        bgAudio.load();
        loose();
    }
    document.getElementById('timebar').style.width = (currentCount/timeAllowed) * 100 + '%';
 
}

function chopsound() {
    document.getElementById("swoosh").load(10);
    setTimeout(function () {
        document.getElementById("swoosh").play();
        setTimeout(function () {
            document.getElementById("swoosh").load(10);
        }, 150);
    }, 0);
}

function scream() {
    document.getElementById("scream").load(50);
    setTimeout(function () {
        document.getElementById("scream").play();
        setTimeout(function () {
            document.getElementById("scream").load(10);
        }, 500);
    }, 0);
}
function win() {
    document.getElementById("win").load(50);
    setTimeout(function () {
        document.getElementById("win").play();
        setTimeout(function () {
            document.getElementById("win").load(10);
        }, 3000);
    }, 0);
}
function loose(){
    document.getElementById("loose").load(50);
    setTimeout(function () {
        document.getElementById("loose").play();
        setTimeout(function () {
            document.getElementById("loose").load(10);
        }, 3000);
    }, 0);
}
