(function(window, document, GUI) {
  "use strict";

  // Aliases
  var assert = console.assert.bind(console),
      sin = Math.sin,
      cos = Math.cos,
      acos = Math.acos,
      atan2 = Math.atan2,
      abs  = Math.abs,
      sqrt = Math.sqrt;

  var M_PI = Math.PI,
      M_PI_2 = M_PI/2,
      M_1_PI = 1/M_PI;

  var MAX_FPS = 35,
      BIG_INT = 50;


  // Globals
  var config = {
    l1: 4.5,
    l2: 8.5,
    beta: 15,
    axis: false,
    zoom: 0.6
  };

  var realZoom, canvas, ctx,
      origin = {x: 0, y: 0},
      target = {x: 0, y: 0};


  window.addEventListener('load', init, false);

  function init() {
    window.addEventListener('resize', throttle(resize, 1000/MAX_FPS|0), false);
    window.addEventListener('mousemove', throttle(move, 1000/MAX_FPS|0), false);
    window.addEventListener('change', update, false);

    canvas = document.querySelector('#area');
    ctx = canvas.getContext('2d');

    resize();

    var gui = new GUI();
    gui.add(config, 'l1', 0.1, 30).step(0.1).name('first link');
    gui.add(config, 'l2', 0.1, 40).step(0.1).name('second link');
    gui.add(config, 'beta', -90, 90).name('shift angle');
    gui.add(config, 'axis').name('show axis');
    gui.add(config, 'zoom', 0.1, 2).step(0.1).onChange(resetFrame);
  }


  function resetFrame() {
    realZoom = config.zoom * 96/2.54;

    ctx.setTransform(1, 0, 0, 1, 0, 0);
    ctx.translate(origin.x, origin.y);
    ctx.scale(realZoom, -realZoom);
  }


  function resize() {
    origin.x = (canvas.width = innerWidth)/2 | 0,
    origin.y = (canvas.height = innerHeight)/2 | 0;

    resetFrame();
    update();
  }


  function move(event) {
    target.x = (event.pageX - origin.x) / realZoom;
    target.y = -(event.pageY - origin.y) / realZoom;

    update();
  }


  function update() {
    ctx.save();
    ctx.setTransform(1, 0, 0, 1, 0, 0);
    ctx.clearRect(0, 0, canvas.width, canvas.height);
    ctx.restore();

    updateIK();
  }


  function updateIK() {
    var l1 = config.l1,
        l2 = config.l2,
        beta = rad(config.beta),
        x = target.x,
        y = target.y,
        l1l1 = l1*l1,
        l2l2 = l2*l2,
        l1ml2l1ml2 = (l2-l1)*(l2-l1),
        l1pl2l1pl2 = (l2+l1)*(l2+l1),
        phi = M_PI_2 - beta,
        sx = sin(beta)*l1,
        sy = cos(beta)*l1;

    var a0, a1, state;
    var r, rr, p;

    rr = x*x + y*y;

    // Far
    if (rr > l1pl2l1pl2) {
      state = 'far';
      a0 = atan2(y, x) + phi;

      if (a0 < 0) {
        a0 = 0;
        a1 = atan2(-y-sy, -x+sx) + phi;
      } else {
        if (M_PI < a0) a0 = M_PI;
        a1 = M_PI;
      }

      return draw(a0, a1, state);
    }

    // Near
    if (rr < l1ml2l1ml2) {
      state = 'near';
      a0 = M_PI;
      a1 = atan2(y-sy, x+sx) + phi;

      if (a1 < 0) {
        a0 = atan2(y, x) + phi;
        if (a0 < 0)
            a0 += M_PI;
        else if (M_PI < a0)
            a0 -= M_PI;
        a1 = 0;
      }
      return draw(a0, a1, state);
    }

    r = sqrt(rr);
    a0 = acos((l1l1 + rr - l2l2)/(2 * l1 * r)) + atan2(x, -y) - beta;

    assert(!isNaN(a0));

    if (a0 < 0) {
      a0 = 0;
      a1 = atan2(-y-sy, -x+sx) + phi;
      if (a1 < 0) {
        state = 'near-3';
        a0 = M_PI;
        a1 = atan2(y-sy, x+sx) + phi;
      } else {
        state = 'far-2';
      }
    } else if (M_PI < a0) {
      state = 'near-2';
      a0 = M_PI;
      a1 = atan2(y-sy, x+sx) + phi;
    } else {
      state = 'simple';
      a1 = acos((l1l1 + l2l2 - rr)/(2 * l1 * l2));
    }

    assert(!isNaN(a0));
    assert(!isNaN(a1));
    assert(0 <= a0 && a0 <= M_PI);
    assert(0 <= a1 && a1 <= M_PI);

    draw(a0, a1, state);
  }


  function draw(a0, a1, state) {
    var l1 = config.l1,
        l2 = config.l2,
        beta = rad(config.beta),
        sx = sin(beta)*l1,
        sy = cos(beta)*l1;

    ctx.strokeStyle = 'black';
    ctx.fillStyle = 'black';
    ctx.lineWidth = 0.1;

    // Region.
    ctx.save();
    ctx.lineWidth = 0.05;
    ctx.fillStyle = '#cf9';
    ctx.strokeStyle = '#900';
    ctx.beginPath();
    ctx.arc(0, 0, l1+l2, -M_PI_2+beta, M_PI_2+beta);
    ctx.arc(-sx, sy, l2, M_PI_2+beta, -M_PI_2+beta, true);
    ctx.arc(0, 0, abs(l2-l1), -M_PI_2+beta, M_PI_2+beta, true);
    ctx.arc(sx, -sy, l2, M_PI_2+beta, -M_PI_2+beta);
    ctx.closePath();
    ctx.fill();
    ctx.stroke();
    ctx.restore();

    // Frame.
    if (config.axis) {
      ctx.save();
      ctx.strokeStyle = '#505050';
      ctx.lineWidth = 0.05;
      drawAxis();
      ctx.setLineDash([0.05, 0.95]);
      ctx.lineDashOffset = -0.5;
      ctx.lineWidth = 0.15;
      drawAxis();
      ctx.lineDashOffset = 0;
      ctx.lineWidth = 0.4;
      drawAxis();
      ctx.restore();
    }

    // Directions.
    ctx.save();
    ctx.lineWidth = 0.1;
    ctx.setLineDash([0.1]);
    ctx.strokeStyle = 'gray';

    ctx.beginPath();
    ctx.arc(0, 0, l1, -M_PI_2+beta, M_PI_2+beta);
    ctx.stroke();

    ctx.rotate(beta);
    ctx.beginPath();
    ctx.moveTo(0, -BIG_INT);
    ctx.lineTo(0, BIG_INT);
    ctx.stroke();

    ctx.rotate(a0);
    ctx.beginPath();
    ctx.moveTo(0, -BIG_INT);
    ctx.lineTo(0, BIG_INT);
    ctx.stroke();
    ctx.restore();

    // Segments.
    var x1 = l1 * cos(a0+beta - M_PI_2),
        y1 = l1 * sin(a0+beta - M_PI_2),
        x2 = x1 + l2 * sin(a0+beta + a1 - M_PI),
        y2 = y1 - l2 * cos(a0+beta + a1 - M_PI);

    ctx.save();
    ctx.lineWidth = 0.08;
    ctx.beginPath();
    ctx.moveTo(0, 0);
    ctx.lineTo(x1, y1);
    ctx.lineTo(x2, y2);
    ctx.stroke();

    ctx.beginPath();
    ctx.arc(0, 0, 0.2, 0, 2*M_PI);
    ctx.fill();
    ctx.beginPath();
    ctx.arc(x1, y1, 0.2, 0, 2*M_PI);
    ctx.fill();
    ctx.beginPath();
    ctx.arc(x2, y2, 0.2, 0, 2*M_PI);
    ctx.fill();
    ctx.restore();

    // Information.
    ctx.save();
    ctx.setTransform(1, 0, 0, 1, 0, 0);
    ctx.font = '12px monospace';
    if (a0 < 0 || M_PI < a0 || a1 < 0 || M_PI < a1)
      ctx.fillStyle = 'red';
    ctx.fillText('Angles: ' + deg(a0).toFixed(0) + '°, '
                            + deg(a1).toFixed(0) + '°', 20, 20);
    ctx.fillText('State: ' + state, 20, 35);
    ctx.fillText('2nd joint: ('+x1.toFixed(1)+', '+y1.toFixed(1)+')', 20, 50);
    ctx.fillText('3rd joint: ('+x2.toFixed(1)+', '+y2.toFixed(1)+')', 20, 65);
    ctx.fillText('Target: (' + target.x.toFixed(1) + ', '
                             + target.y.toFixed(1) + ')', 20, 80);
    ctx.restore();
  }


  function drawAxis() {
    ctx.beginPath()
    ctx.moveTo(0, -BIG_INT);
    ctx.lineTo(0, BIG_INT);
    ctx.stroke();

    ctx.beginPath()
    ctx.moveTo(-BIG_INT, 0);
    ctx.lineTo(BIG_INT, 0);
    ctx.stroke();
  }


  function throttle(fn, ms) {
    var state = 0, self, args;

    function later() {
      if (state == 1) {
        state = 0;
      } else if (state == 2) {
        fn.apply(self, args);
        state = 1;
        setTimeout(later, ms);
      }
    }

    return function() {
      if (state == 0) {
        fn.apply(this, arguments);
        state = 1;
        setTimeout(later, ms);
      } else {
        self = this;
        args = arguments;
        state = 2;
      }
    };
  }


  function rad(deg) { return deg * M_PI/180; }
  function deg(rad) { return rad * 180*M_1_PI; }

})(window, document, dat.GUI);
