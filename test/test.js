var h = 0;
var m = 0;

var onH = 7;
var onM = 0;
var offH = 23;
var offM = 00;

function tik() {
    m += 1;

    if (m > 59) {
        m = 0;
        h += 1;
    }

    if (h > 23) {
        h = 0;
    }
}

function checkRightDirection(h1, h2, m1, m2)
{
  if(h1 == h2) {
    return m1 < m2;
  } else {
    return h1 < h2;
  }
}


function isMore(currentH, currentM, h, m)
{
  if (currentH >= h) {
    if (currentM >= m) {
      return true;
    } else {
      return false;
    }
  } else {
    return false;
  }
}

var t1 = false;
var t2 = true;

setInterval(function() {
    tik();
    if (checkRightDirection(onH, offH, onM, offM)) {
        if (isMore(h, m, onH, onM) && !isMore(h, m, offH, offM)) {
            // if (!t1) {
            //     t1 = true;
            //     t2 = false;
            //     console.log('on', h, m, onH, onM);
            // }
            console.log('on');
        } else {
            // if (!t2) {
            //     t2 = true;
            //     t1 = false;
            //     console.log('off', h, m, offH, offM);
            // }
            console.log('off');
        }
    } 
}, 20);

// console.log(isMore(7, 1, onH, onM));