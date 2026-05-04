document.addEventListener('DOMContentLoaded', () => {

  // === LOADING ===
  setTimeout(() => document.getElementById('loading').classList.add('hidden'), 2000);

  // === DUST PARTICLES ===
  const dustContainer = document.querySelector('.dust-container');
  for (let i = 0; i < 25; i++) {
    const d = document.createElement('div');
    d.classList.add('dust');
    d.style.left = Math.random() * 100 + '%';
    d.style.animationDuration = (6 + Math.random() * 8) + 's';
    d.style.animationDelay = (Math.random() * 10) + 's';
    d.style.width = d.style.height = (1 + Math.random() * 2) + 'px';
    dustContainer.appendChild(d);
  }

  // === BOOK COVER ===
  const cover = document.querySelector('.book-cover');
  const book = document.querySelector('.book');
  let bookOpen = false;

  cover.addEventListener('click', () => {
    if (bookOpen) return;
    bookOpen = true;
    cover.classList.add('open');
    setTimeout(() => {
      document.querySelector('.spreads-container').style.pointerEvents = 'auto';
      activateSpread(0);
    }, 600);
  });

  // === SPREAD NAVIGATION ===
  const spreads = document.querySelectorAll('.spread');
  const dots = document.querySelectorAll('.nav-dot');
  const tabs = document.querySelectorAll('.chapter-tab');
  const prevBtn = document.getElementById('nav-prev');
  const nextBtn = document.getElementById('nav-next');
  let currentSpread = 0;
  let isAnimating = false;

  function activateSpread(index, direction = 'next') {
    if (isAnimating || index === currentSpread || index < 0 || index >= spreads.length) return;
    isAnimating = true;

    // Page turn overlay
    const overlay = document.createElement('div');
    overlay.className = 'page-turning';
    const turnPage = document.createElement('div');
    turnPage.className = direction === 'next' ? 'page-turn-overlay' : 'page-turn-overlay reverse';
    overlay.appendChild(turnPage);
    document.querySelector('.spreads-container').appendChild(overlay);

    setTimeout(() => {
      spreads[currentSpread].classList.remove('active');
      spreads[index].classList.add('active');
      currentSpread = index;
      updateNav();
      animateContent();
    }, 350);

    setTimeout(() => {
      overlay.remove();
      isAnimating = false;
    }, 700);
  }

  function updateNav() {
    prevBtn.disabled = currentSpread === 0;
    nextBtn.disabled = currentSpread === spreads.length - 1;
    dots.forEach((d, i) => d.classList.toggle('active', i === currentSpread));
    tabs.forEach((t, i) => t.classList.toggle('active', i === currentSpread));
  }

  function animateContent() {
    const active = spreads[currentSpread];
    const els = active.querySelectorAll('.fade-in');
    els.forEach((el, i) => {
      el.classList.remove('show');
      setTimeout(() => el.classList.add('show'), 100 + i * 80);
    });
    // Animate stat counters if on stats spread
    active.querySelectorAll('.stat-val[data-target]').forEach(counter => {
      if (counter.dataset.done) return;
      counter.dataset.done = '1';
      const target = parseInt(counter.dataset.target);
      const suffix = counter.dataset.suffix || '';
      const start = performance.now();
      const duration = 1800;
      (function tick(now) {
        const p = Math.min((now - start) / duration, 1);
        const eased = 1 - Math.pow(1 - p, 3);
        counter.textContent = Math.floor(target * eased) + suffix;
        if (p < 1) requestAnimationFrame(tick);
      })(start);
    });
  }

  prevBtn.addEventListener('click', () => activateSpread(currentSpread - 1, 'prev'));
  nextBtn.addEventListener('click', () => activateSpread(currentSpread + 1, 'next'));
  dots.forEach((d, i) => d.addEventListener('click', () => {
    activateSpread(i, i > currentSpread ? 'next' : 'prev');
  }));
  tabs.forEach((t, i) => t.addEventListener('click', () => {
    if (!bookOpen) { cover.click(); setTimeout(() => activateSpread(i), 1400); return; }
    activateSpread(i, i > currentSpread ? 'next' : 'prev');
  }));

  // Keyboard nav
  document.addEventListener('keydown', e => {
    if (!bookOpen) { if (e.key === 'Enter' || e.key === ' ') cover.click(); return; }
    if (e.key === 'ArrowRight' || e.key === 'ArrowDown') activateSpread(currentSpread + 1, 'next');
    if (e.key === 'ArrowLeft' || e.key === 'ArrowUp') activateSpread(currentSpread - 1, 'prev');
  });

  // Touch/swipe
  let touchStart = 0;
  document.addEventListener('touchstart', e => { touchStart = e.touches[0].clientX; });
  document.addEventListener('touchend', e => {
    if (!bookOpen) return;
    const diff = touchStart - e.changedTouches[0].clientX;
    if (Math.abs(diff) > 50) {
      activateSpread(currentSpread + (diff > 0 ? 1 : -1), diff > 0 ? 'next' : 'prev');
    }
  });

  // === CURSOR TRAIL (golden ink drops) ===
  let lastTrail = 0;
  document.addEventListener('mousemove', e => {
    const now = Date.now();
    if (now - lastTrail < 80) return;
    lastTrail = now;
    const dot = document.createElement('div');
    dot.style.cssText = `position:fixed;left:${e.clientX}px;top:${e.clientY}px;
      width:3px;height:3px;background:#c9a84c;border-radius:50%;
      pointer-events:none;z-index:9998;opacity:0.5;
      transition:all 0.5s ease;`;
    document.body.appendChild(dot);
    requestAnimationFrame(() => { dot.style.opacity = '0'; dot.style.transform = 'scale(0) translateY(-8px)'; });
    setTimeout(() => dot.remove(), 500);
  });

  // === PROJECT CARDS TILT ===
  document.querySelectorAll('.project-entry').forEach(card => {
    card.addEventListener('mousemove', e => {
      const r = card.getBoundingClientRect();
      const x = (e.clientX - r.left) / r.width - 0.5;
      const y = (e.clientY - r.top) / r.height - 0.5;
      card.style.transform = `translateY(-2px) perspective(600px) rotateX(${y * -3}deg) rotateY(${x * 3}deg)`;
    });
    card.addEventListener('mouseleave', () => { card.style.transform = ''; });
  });

  // Init nav state
  updateNav();
});
