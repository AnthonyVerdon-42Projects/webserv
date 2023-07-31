var xMax = document.body.clientWidth - 200;
var yMax = document.body.clientHeight - 200;

var x = Math.random() * xMax;
var y = Math.random() * yMax;
var xSpeed = 2 * (Math.random() < 0.5 ? -1 : 1);
var ySpeed = 2 * (Math.random() < 0.5 ? -1 : 1);

var audio1 = new Audio('https://www.myinstants.com/media/sounds/aughhhhh-aughhhhh.mp3');
var audio2 = new Audio('https://www.myinstants.com/media/sounds/among-us-role-reveal-sound.mp3');
var audio3 = new Audio('https://www.myinstants.com/media/sounds/deja-vu.mp3');
var audio4 = new Audio('https://www.myinstants.com/media/sounds/bonk_7zPAD7C.mp3');

var screamer = document.createElement('img');
screamer.src = 'averdon.png';

var stop = false;
var stuck = false;

document.body.style.backgroundImage = 'url("chriscelaya.jpg")';

addEventListener('resize', function() {
	xMax = document.body.clientWidth - 200;
	yMax = document.body.clientHeight - 200;
	if (x > xMax)
		x = xMax;
	if (y > yMax)
		y = yMax;
	document.body.style.backgroundPositionX = x + 'px';
	document.body.style.backgroundPositionY = y + window.scrollY + 'px';
});

addEventListener('scroll', function() {
	yMax = document.body.clientHeight - 200;
	if (y > yMax)
		y = yMax;
	this.document.body.style.backgroundPositionY = y + window.scrollY + 'px';
});

addEventListener('click', function(event) {
	if (stop || stuck)
		return;

	if (event.clientX > x && event.clientX < x + 200 && event.clientY > y && event.clientY < y + 200) {
		if (document.body.style.backgroundImage == 'url("chriscelaya.jpg")') {
			audio1.currentTime = 0.5;
			audio1.play();
			document.body.style.animation = 'shake 0.5s linear infinite';
			stop = true;
		}
		else if (document.body.style.backgroundImage == 'url("averdon.png")') {
			audio2.currentTime = 0.5;
			audio2.play();
			document.body.appendChild(screamer);
			stop = true;
		}
		else if (document.body.style.backgroundImage == 'url("tguerin.png")') {
			audio3.currentTime = 0.5;
			audio3.play();
			xSpeed *= 10;
			ySpeed *= 10;
			stuck = true;
		}
		else if (document.body.style.backgroundImage == 'url("ulayus.png")') {
			audio4.currentTime = 0.0;
			audio4.play();
		}
	}
});

audio1.addEventListener('ended', function() {
	document.body.style.animation = '';
	stop = false;
});

audio2.addEventListener('ended', function() {
	screamer.remove();
	stop = false;
});

audio3.addEventListener('ended', function() {
	xSpeed /= 10;
	ySpeed /= 10;
	stuck = false;
});

audio4.addEventListener('ended', function() {
	if (document.body.style.backgroundImage != 'url("ulayus.png")')
		return;

	document.body.style.backgroundSize = '200px 50px';
	y += 150;
	if (y > yMax)
		y = yMax;
	document.body.style.backgroundPositionY = y + window.scrollY + 'px';
});

function move() {
	if (stop)
		return;

	x += xSpeed;
	y += ySpeed;
	if (x > xMax) {
		x = xMax;
		xSpeed = -xSpeed;
		if (!stuck)
			document.body.style.backgroundImage = 'url("averdon.png")';
		document.body.style.backgroundSize = '200px 200px';
	}
	if (x < 0) {
		x = 0;
		xSpeed = -xSpeed;
		if (!stuck)
			document.body.style.backgroundImage = 'url("tguerin.png")';
		document.body.style.backgroundSize = '200px 200px';
	}
	if (y > yMax) {
		y = yMax;
		ySpeed = -ySpeed;
		if (!stuck)
			document.body.style.backgroundImage = 'url("ulayus.png")';
	}
	if (y < 0) {
		y = 0;
		ySpeed = -ySpeed;
		if (!stuck)
			document.body.style.backgroundImage = 'url("chriscelaya.jpg")';
		document.body.style.backgroundSize = '200px 200px';
	}
	document.body.style.backgroundPositionX = x + 'px';
	document.body.style.backgroundPositionY = y + window.scrollY + 'px';
}

setInterval(move, 10);
