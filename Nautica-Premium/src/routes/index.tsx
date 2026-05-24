import { createFileRoute, Link } from "@tanstack/react-router";
import { ArrowRight, ShieldCheck, Sparkles, TrendingUp, Zap, Globe, Lock, ChevronDown } from "lucide-react";
import { useEffect, useRef, useState } from "react";
import logo from "@/assets/nautica-logo.png";

export const Route = createFileRoute("/")(
  {
    head: () => ({
      meta: [
        { title: "Nautica — Premium NFT Marketplace" },
        {
          name: "description",
          content:
            "Discover, collect, create and trade on-chain art with zero-knowledge privacy. Welcome to Nautica.",
        },
        { property: "og:title", content: "Nautica" },
        {
          property: "og:description",
          content:
            "The premium NFT marketplace. Collect on-chain art. Send privately with zero-knowledge proofs.",
        },
      ],
    }),
    component: SplashPage,
  },
);

/* ── Animated Counter ── */
function AnimatedNumber({ target, suffix = "" }: { target: string; suffix?: string }) {
  const [display, setDisplay] = useState("0");
  const ref = useRef<HTMLSpanElement>(null);

  useEffect(() => {
    const num = parseFloat(target.replace(/,/g, ""));
    const hasComma = target.includes(",");
    const duration = 1600;
    const start = performance.now();

    function step(now: number) {
      const elapsed = now - start;
      const progress = Math.min(elapsed / duration, 1);
      const eased = 1 - Math.pow(1 - progress, 3); // ease-out cubic
      const current = num * eased;
      if (hasComma) {
        setDisplay(Math.floor(current).toLocaleString());
      } else if (target.includes(".")) {
        setDisplay(current.toFixed(1));
      } else {
        setDisplay(Math.floor(current).toLocaleString());
      }
      if (progress < 1) requestAnimationFrame(step);
    }

    const observer = new IntersectionObserver(
      ([entry]) => {
        if (entry.isIntersecting) {
          requestAnimationFrame(step);
          observer.disconnect();
        }
      },
      { threshold: 0.3 },
    );
    if (ref.current) observer.observe(ref.current);
    return () => observer.disconnect();
  }, [target]);

  return (
    <span ref={ref}>
      {display}
      {suffix}
    </span>
  );
}

/* ── Floating Particle Canvas ── */
function ParticleField() {
  const canvasRef = useRef<HTMLCanvasElement>(null);

  useEffect(() => {
    const canvas = canvasRef.current;
    if (!canvas) return;
    const ctx = canvas.getContext("2d");
    if (!ctx) return;

    let animId: number;
    const particles: { x: number; y: number; vx: number; vy: number; r: number; a: number }[] = [];

    function resize() {
      canvas!.width = canvas!.offsetWidth * 2;
      canvas!.height = canvas!.offsetHeight * 2;
    }
    resize();
    window.addEventListener("resize", resize);

    for (let i = 0; i < 40; i++) {
      particles.push({
        x: Math.random() * canvas.width,
        y: Math.random() * canvas.height,
        vx: (Math.random() - 0.5) * 0.4,
        vy: (Math.random() - 0.5) * 0.4,
        r: Math.random() * 2 + 0.5,
        a: Math.random() * 0.3 + 0.1,
      });
    }

    function draw() {
      ctx!.clearRect(0, 0, canvas!.width, canvas!.height);
      for (const p of particles) {
        p.x += p.vx;
        p.y += p.vy;
        if (p.x < 0) p.x = canvas!.width;
        if (p.x > canvas!.width) p.x = 0;
        if (p.y < 0) p.y = canvas!.height;
        if (p.y > canvas!.height) p.y = 0;

        ctx!.beginPath();
        ctx!.arc(p.x, p.y, p.r, 0, Math.PI * 2);
        ctx!.fillStyle = `rgba(115, 220, 190, ${p.a})`;
        ctx!.fill();
      }
      // Draw faint connections
      for (let i = 0; i < particles.length; i++) {
        for (let j = i + 1; j < particles.length; j++) {
          const dx = particles[i].x - particles[j].x;
          const dy = particles[i].y - particles[j].y;
          const dist = Math.sqrt(dx * dx + dy * dy);
          if (dist < 150) {
            ctx!.beginPath();
            ctx!.moveTo(particles[i].x, particles[i].y);
            ctx!.lineTo(particles[j].x, particles[j].y);
            ctx!.strokeStyle = `rgba(115, 220, 190, ${0.06 * (1 - dist / 150)})`;
            ctx!.stroke();
          }
        }
      }
      animId = requestAnimationFrame(draw);
    }
    draw();

    return () => {
      cancelAnimationFrame(animId);
      window.removeEventListener("resize", resize);
    };
  }, []);

  return (
    <canvas
      ref={canvasRef}
      className="absolute inset-0 w-full h-full pointer-events-none"
      style={{ opacity: 0.6 }}
    />
  );
}

const STATS = [
  { label: "Total Volume", value: "1,247", unit: "ETH", icon: TrendingUp },
  { label: "NFTs Created", value: "8,432", unit: "", icon: Sparkles },
  { label: "Active Artists", value: "312", unit: "", icon: Globe },
  { label: "ZK Proofs", value: "98.4", unit: "k", icon: ShieldCheck },
];

const FEATURES = [
  {
    icon: Sparkles,
    title: "Create & Mint",
    desc: "Multi-step wizard with trait builder, royalty settings, and instant on-chain deployment.",
    color: "oklch(0.72 0.16 180)",
  },
  {
    icon: Lock,
    title: "Private Payments",
    desc: "ZK-proven transfers. The network verifies, the world doesn't see your ledger.",
    color: "oklch(0.78 0.12 65)",
  },
  {
    icon: Zap,
    title: "Instant Swaps",
    desc: "Trade tokens across chains with best-route aggregation and MEV protection.",
    color: "oklch(0.62 0.22 25)",
  },
];

const ARTISTS = [
  "Anouk Vey", "Hiro Tanaka", "Mara Idris", "Lev Marin",
  "P. Okafor", "Sela Kim", "J. Moreau", "Ada Chen", "Kai Stone",
];

/* ── Marquee component ── */
function Marquee({ children }: { children: React.ReactNode }) {
  return (
    <div className="overflow-hidden relative">
      <div className="flex gap-4 animate-marquee whitespace-nowrap">
        {children}
        {children}
      </div>
    </div>
  );
}

function SplashPage() {
  const [mousePos, setMousePos] = useState({ x: 0.5, y: 0.5 });

  function handleMouseMove(e: React.MouseEvent) {
    const rect = e.currentTarget.getBoundingClientRect();
    setMousePos({
      x: (e.clientX - rect.left) / rect.width,
      y: (e.clientY - rect.top) / rect.height,
    });
  }

  return (
    <div
      className="min-h-screen bg-background relative overflow-hidden"
      onMouseMove={handleMouseMove}
    >
      {/* Particle canvas */}
      <ParticleField />

      {/* Mouse-following glow */}
      <div
        aria-hidden
        className="absolute w-[600px] h-[600px] rounded-full opacity-15 blur-[120px] pointer-events-none transition-all duration-[2000ms] ease-out"
        style={{
          left: `${mousePos.x * 100}%`,
          top: `${mousePos.y * 100}%`,
          transform: "translate(-50%, -50%)",
          background:
            "radial-gradient(circle, oklch(0.72 0.16 180 / 0.5), transparent 70%)",
        }}
      />
      <div
        aria-hidden
        className="absolute bottom-[-10%] right-[-10%] w-[500px] h-[500px] rounded-full opacity-12 blur-[100px]"
        style={{
          background:
            "radial-gradient(circle, oklch(0.78 0.12 65 / 0.4), transparent 70%)",
        }}
      />

      <div className="relative z-10 mx-auto max-w-6xl px-5 md:px-10 py-6 md:py-10">
        {/* Top bar */}
        <header className="flex items-center justify-between gap-3 animate-fade-down">
          <div className="flex items-center gap-3">
            <div className="w-11 h-11 rounded-2xl bg-primary/10 border border-primary/20 grid place-items-center overflow-hidden group hover:shadow-glow-primary transition-shadow duration-500">
              <img src={logo} alt="" width={32} height={32} className="w-8 h-8 group-hover:scale-110 transition-transform duration-300" />
            </div>
            <div>
              <span className="font-display text-xl font-bold text-ink">Nautica</span>
              <span className="hidden sm:block text-[10px] text-muted-foreground font-medium tracking-wider uppercase">
                Premium Marketplace
              </span>
            </div>
          </div>
          <div className="flex items-center gap-3">
            <Link to="/market" className="btn-ghost text-xs hidden sm:flex">Explore</Link>
            <Link to="/create" className="btn-ghost text-xs hidden sm:flex">Create</Link>
            <Link to="/market" className="btn-cta text-xs">
              Launch App <ArrowRight className="w-3.5 h-3.5" />
            </Link>
          </div>
        </header>

        {/* Hero */}
        <div className="mt-12 md:mt-20 grid lg:grid-cols-[1.3fr_1fr] gap-8 items-center">
          <div className="animate-fade-up">
            <span className="inline-flex items-center gap-2 px-3 py-1.5 rounded-full bg-primary/10 border border-primary/20 text-primary text-xs font-display font-semibold animate-pulse-glow">
              <Zap className="w-3 h-3" />
              v2.0 · Create, Trade, Send Privately
            </span>

            <h1 className="font-display text-5xl sm:text-6xl md:text-7xl font-bold mt-6 leading-[1.05] text-ink">
              The future of
              <br />
              <span
                className="animate-gradient bg-clip-text text-transparent"
                style={{
                  backgroundImage:
                    "linear-gradient(135deg, oklch(0.72 0.16 180), oklch(0.78 0.12 65), oklch(0.62 0.22 25), oklch(0.72 0.16 180))",
                  backgroundSize: "300% 300%",
                }}
              >
                digital art
              </span>
              <br />
              starts here.
            </h1>

            <p className="mt-5 max-w-lg text-base md:text-lg text-muted-foreground leading-relaxed">
              A premium marketplace for curated digital editions — paired with
              ZK-proven private payments and multi-chain token swaps that rival
              the best wallets.
            </p>

            <div className="mt-8 flex flex-wrap gap-3">
              <Link to="/market" className="btn-cta group">
                Explore Market <ArrowRight className="w-4 h-4 group-hover:translate-x-1 transition-transform" />
              </Link>
              <Link to="/create" className="btn-secondary group">
                <PlusIcon className="w-4 h-4 group-hover:rotate-90 transition-transform duration-300" /> Create NFT
              </Link>
            </div>

            {/* Scroll indicator */}
            <div className="mt-12 hidden md:flex items-center gap-2 text-muted-foreground/50 animate-bounce" style={{ animationDuration: "2s" }}>
              <ChevronDown className="w-4 h-4" />
              <span className="text-xs font-display font-medium">Scroll to explore</span>
            </div>
          </div>

          {/* Hero visual — stacked floating cards with parallax */}
          <div className="relative hidden lg:block animate-scale-in">
            <div className="relative w-full aspect-square max-w-[420px] mx-auto">
              {/* Orbiting glow */}
              <div className="absolute inset-0 animate-spin-slow" style={{ animationDuration: "30s" }}>
                <div className="absolute top-0 left-1/2 w-3 h-3 rounded-full bg-primary/40 blur-sm" />
              </div>
              <div className="absolute inset-0 animate-spin-slow" style={{ animationDuration: "45s", animationDirection: "reverse" }}>
                <div className="absolute bottom-0 right-1/4 w-2 h-2 rounded-full bg-highlight/40 blur-sm" />
              </div>

              {/* Card 1 */}
              <div
                className="absolute top-[5%] left-[5%] w-[65%] aspect-[3/4] nautica-glass rounded-3xl overflow-hidden shadow-elevated animate-float"
                style={{
                  animationDelay: "0s",
                  transform: `translateX(${(mousePos.x - 0.5) * -8}px) translateY(${(mousePos.y - 0.5) * -8}px)`,
                }}
              >
                <div className="absolute inset-0" style={{ backgroundImage: "linear-gradient(135deg, oklch(0.50 0.20 180), oklch(0.35 0.15 220))" }} />
                <div className="absolute bottom-0 left-0 right-0 p-4 bg-gradient-to-t from-black/60 to-transparent">
                  <p className="text-white/80 text-[10px] font-medium">Anouk Vey</p>
                  <p className="text-white font-display font-semibold text-sm">Tideglass 04</p>
                  <p className="text-white/70 text-xs mt-1">0.42 ETH</p>
                </div>
              </div>

              {/* Card 2 */}
              <div
                className="absolute top-[15%] right-[5%] w-[55%] aspect-[3/4] nautica-glass rounded-3xl overflow-hidden shadow-elevated animate-float"
                style={{
                  animationDelay: "1s",
                  transform: `translateX(${(mousePos.x - 0.5) * 6}px) translateY(${(mousePos.y - 0.5) * 6}px)`,
                }}
              >
                <div className="absolute inset-0" style={{ backgroundImage: "linear-gradient(135deg, oklch(0.55 0.22 340), oklch(0.38 0.25 300))" }} />
                <div className="absolute bottom-0 left-0 right-0 p-4 bg-gradient-to-t from-black/60 to-transparent">
                  <p className="text-white/80 text-[10px] font-medium">Ada Chen</p>
                  <p className="text-white font-display font-semibold text-sm">Neural Bloom</p>
                  <p className="text-white/70 text-xs mt-1">2.10 ETH</p>
                </div>
              </div>

              {/* Card 3 */}
              <div
                className="absolute bottom-[5%] left-[15%] w-[50%] aspect-[3/4] nautica-glass rounded-3xl overflow-hidden shadow-elevated animate-float"
                style={{
                  animationDelay: "2s",
                  transform: `translateX(${(mousePos.x - 0.5) * -4}px) translateY(${(mousePos.y - 0.5) * 4}px)`,
                }}
              >
                <div className="absolute inset-0" style={{ backgroundImage: "linear-gradient(135deg, oklch(0.62 0.16 155), oklch(0.42 0.14 195))" }} />
                <div className="absolute bottom-0 left-0 right-0 p-4 bg-gradient-to-t from-black/60 to-transparent">
                  <p className="text-white/80 text-[10px] font-medium">J. Moreau</p>
                  <p className="text-white font-display font-semibold text-sm">Pulse Sequence</p>
                  <p className="text-white/70 text-xs mt-1">0.065 ETH</p>
                </div>
              </div>

              {/* Floating price tag */}
              <div
                className="absolute top-[60%] right-[0%] nautica-glass px-4 py-3 rounded-2xl animate-float shadow-pop"
                style={{
                  animationDelay: "0.5s",
                  transform: `translateX(${(mousePos.x - 0.5) * 10}px) translateY(${(mousePos.y - 0.5) * -5}px)`,
                }}
              >
                <p className="text-[10px] text-muted-foreground font-medium">Floor Price</p>
                <p className="font-display text-xl font-bold text-ink">
                  0.38 <span className="text-xs text-primary font-medium">ETH</span>
                </p>
                <span className="badge-success text-[9px] mt-1">+12.5%</span>
              </div>
            </div>
          </div>
        </div>

        {/* Stats with animated counters */}
        <section className="mt-16 md:mt-20 grid grid-cols-2 md:grid-cols-4 gap-3 stagger-children">
          {STATS.map((s) => (
            <div
              key={s.label}
              className="stat-card animate-fade-up group hover:border-primary/20 hover:shadow-glow-primary/10 transition-all duration-500 cursor-default"
            >
              <div className="flex items-center gap-2 mb-3">
                <div className="w-8 h-8 rounded-xl bg-primary/10 grid place-items-center group-hover:bg-primary/20 group-hover:scale-110 transition-all duration-300">
                  <s.icon className="w-4 h-4 text-primary" />
                </div>
                <p className="text-[10px] uppercase tracking-widest text-muted-foreground font-semibold">{s.label}</p>
              </div>
              <p className="font-display text-3xl md:text-4xl font-bold text-ink leading-none">
                <AnimatedNumber target={s.value} suffix={s.unit ? ` ` : ""} />
                {s.unit && <span className="text-sm text-muted-foreground ml-1 font-medium">{s.unit}</span>}
              </p>
            </div>
          ))}
        </section>

        {/* Scrolling ticker */}
        <section className="mt-8 overflow-hidden">
          <Marquee>
            {["Tideglass 04 — 0.42 ETH", "Neural Bloom — 2.10 ETH", "Grove, August — 0.32 ETH", "Folded Light — 0.18 ETH", "Pulse Sequence — 0.065 ETH", "Reef Memory — 0.08 ETH"].map((t) => (
              <span key={t} className="flex items-center gap-2 text-xs text-muted-foreground/60 font-display font-medium">
                <span className="w-1.5 h-1.5 rounded-full bg-primary/40" />
                {t}
              </span>
            ))}
          </Marquee>
        </section>

        {/* Features */}
        <section className="mt-12 grid md:grid-cols-3 gap-4 stagger-children">
          {FEATURES.map((f) => (
            <div
              key={f.title}
              className="nautica-glass p-6 animate-fade-up group hover:border-primary/20 transition-all duration-500 relative overflow-hidden cursor-default"
            >
              <div
                aria-hidden
                className="absolute -top-12 -right-12 w-32 h-32 rounded-full opacity-10 blur-3xl group-hover:opacity-25 group-hover:scale-110 transition-all duration-700"
                style={{ background: f.color }}
              />
              <div className="relative">
                <div
                  className="w-12 h-12 rounded-2xl grid place-items-center mb-4 group-hover:scale-110 transition-transform duration-300"
                  style={{ background: `${f.color}20` }}
                >
                  <f.icon className="w-5 h-5" style={{ color: f.color }} />
                </div>
                <h3 className="font-display text-xl font-bold text-ink">{f.title}</h3>
                <p className="text-sm text-muted-foreground mt-2 leading-relaxed">{f.desc}</p>
                <div className="mt-4 flex items-center gap-1 text-primary text-xs font-display font-semibold opacity-0 group-hover:opacity-100 translate-y-2 group-hover:translate-y-0 transition-all duration-300">
                  Learn more <ArrowRight className="w-3 h-3" />
                </div>
              </div>
            </div>
          ))}
        </section>

        {/* Artists */}
        <section className="mt-12 nautica-glass p-6 overflow-hidden animate-fade-up">
          <div className="flex items-center gap-2 text-muted-foreground mb-4">
            <TrendingUp className="w-4 h-4 text-primary" />
            <span className="text-xs uppercase tracking-widest font-display font-semibold">Featured Artists</span>
          </div>
          <div className="flex flex-wrap gap-2">
            {ARTISTS.map((a) => (
              <span
                key={a}
                className="nautica-pill hover:bg-primary/10 hover:text-primary hover:border-primary/20 border border-transparent cursor-pointer transition-all duration-200 hover:scale-105"
              >
                {a}
              </span>
            ))}
          </div>
        </section>

        {/* CTA */}
        <section className="mt-12 mb-8 nautica-card-feature p-8 md:p-12 text-center relative overflow-hidden">
          <div aria-hidden className="absolute inset-0 dot-grid opacity-10" />
          <div className="relative">
            <h2
              className="font-display text-3xl md:text-4xl font-bold animate-gradient bg-clip-text text-transparent"
              style={{
                backgroundImage: "linear-gradient(135deg, oklch(0.92 0.04 280), oklch(0.78 0.12 65), oklch(0.92 0.04 280))",
                backgroundSize: "200% 200%",
              }}
            >
              Ready to dive in?
            </h2>
            <p className="mt-3 text-feature-foreground/70 max-w-md mx-auto">
              Join the creators and collectors shaping the future of on-chain art.
            </p>
            <div className="mt-6 flex justify-center gap-3">
              <Link to="/market" className="btn-cta group">
                Enter Marketplace <ArrowRight className="w-4 h-4 group-hover:translate-x-1 transition-transform" />
              </Link>
            </div>
          </div>
        </section>
      </div>
    </div>
  );
}

function PlusIcon({ className }: { className?: string }) {
  return (
    <svg className={className} viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth={2} strokeLinecap="round">
      <line x1="12" y1="5" x2="12" y2="19" />
      <line x1="5" y1="12" x2="19" y2="12" />
    </svg>
  );
}
