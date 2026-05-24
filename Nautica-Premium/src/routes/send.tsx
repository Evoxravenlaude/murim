import { createFileRoute } from "@tanstack/react-router";
import { useMutation, useQuery } from "@tanstack/react-query";
import { useEffect, useState } from "react";
import { useServerFn } from "@tanstack/react-start";
import { ShieldCheck, Lock, ArrowRight, Check, Loader2, Zap } from "lucide-react";
import { AppLayout } from "@/components/nautica/AppLayout";
import { getProofStatus, submitPayment } from "@/lib/nautica.functions";

export const Route = createFileRoute("/send")(
  {
    head: () => ({
      meta: [
        { title: "Send — Nautica" },
        { name: "description", content: "Send private, ZK-proven payments on Nautica. Verified, not surveilled." },
        { property: "og:title", content: "Send — Nautica" },
      ],
    }),
    component: SendPage,
  },
);

function SendPage() {
  const [to, setTo] = useState("");
  const [amount, setAmount] = useState("0.05");
  const [currency, setCurrency] = useState<"ETH" | "USDC" | "SOL">("ETH");
  const [note, setNote] = useState("");
  const [jobId, setJobId] = useState<string | null>(null);

  const submitFn = useServerFn(submitPayment);
  const m = useMutation({
    mutationFn: (vars: { to: string; amount: number; currency: "ETH" | "USDC" | "SOL"; note: string }) =>
      submitFn({ data: vars }),
    onSuccess: (r) => setJobId(r.jobId),
  });

  const statusFn = useServerFn(getProofStatus);
  const { data: status } = useQuery({
    queryKey: ["proof", jobId],
    queryFn: () => statusFn({ data: { jobId: jobId! } }),
    enabled: !!jobId,
    refetchInterval: (q) => (q.state.data?.status === "verified" ? false : 1000),
  });

  useEffect(() => {
    if (status?.status === "verified") {
      // celebration
    }
  }, [status?.status]);

  const STEPS = [
    { key: "queued", label: "Queued in prover", desc: "Transaction entered the ZK queue" },
    { key: "proving", label: "Generating SP1 proof", desc: "Creating zero-knowledge proof" },
    { key: "verified", label: "Verified on-chain", desc: "Proof validated and settled" },
  ] as const;

  return (
    <AppLayout>
      <header className="animate-fade-down">
        <p className="text-xs text-primary font-display font-semibold uppercase tracking-widest">Private Payments</p>
        <h1 className="font-display text-4xl md:text-5xl font-bold text-ink mt-1">Send</h1>
      </header>

      <div className="mt-6 grid lg:grid-cols-[1.2fr_1fr] gap-5">
        {/* Composer */}
        <section className="nautica-glass p-6 md:p-8 animate-fade-up">
          <div className="flex items-center gap-2 text-muted-foreground">
            <Lock className="w-4 h-4 text-primary" />
            <span className="text-xs uppercase tracking-widest font-display font-semibold">Encrypted Draft</span>
          </div>

          <label className="block mt-6">
            <span className="text-sm text-muted-foreground font-display font-medium">Recipient</span>
            <input
              value={to}
              onChange={(e) => setTo(e.target.value)}
              placeholder="0x… or nautica.eth"
              className="nautica-input mt-2"
            />
          </label>

          {/* Amount section */}
          <div className="mt-5 rounded-2xl bg-primary/5 border border-primary/10 p-5">
            <p className="text-xs uppercase tracking-widest text-primary font-display font-semibold">Amount</p>
            <div className="flex items-end gap-3 mt-2">
              <input
                value={amount}
                onChange={(e) => setAmount(e.target.value)}
                inputMode="decimal"
                className="bg-transparent font-display text-5xl font-bold text-ink outline-none w-full min-w-0"
              />
              <div className="flex rounded-full bg-muted p-1 text-sm shrink-0">
                {(["ETH", "USDC", "SOL"] as const).map((c) => (
                  <button
                    key={c}
                    onClick={() => setCurrency(c)}
                    className={
                      "px-3.5 py-2 rounded-full font-display font-semibold text-xs transition-all duration-200 " +
                      (currency === c
                        ? "bg-primary text-primary-foreground shadow-soft"
                        : "text-muted-foreground hover:text-foreground")
                    }
                  >
                    {c}
                  </button>
                ))}
              </div>
            </div>
            <p className="text-xs text-muted-foreground mt-2">
              ≈ ${currency === "ETH" ? (Number(amount) * 3847.52).toFixed(2) : currency === "SOL" ? (Number(amount) * 178.9).toFixed(2) : amount}
            </p>
          </div>

          <label className="block mt-5">
            <span className="text-sm text-muted-foreground font-display font-medium">Note (encrypted, optional)</span>
            <input
              value={note}
              onChange={(e) => setNote(e.target.value)}
              placeholder="For the print, August"
              className="nautica-input mt-2"
            />
          </label>

          <button
            disabled={m.isPending || !to || !amount}
            onClick={() => m.mutate({ to, amount: Number(amount), currency, note })}
            className="mt-6 w-full btn-cta py-4"
          >
            {m.isPending ? (
              <><Loader2 className="w-4 h-4 animate-spin" /> Generating proof…</>
            ) : (
              <><ShieldCheck className="w-4 h-4" /> Send Privately</>
            )}
          </button>
        </section>

        {/* Proof status */}
        <aside className="nautica-glass p-6 md:p-8 flex flex-col animate-fade-up" style={{ animationDelay: "100ms" }}>
          <div className="flex items-center gap-2 text-muted-foreground">
            <ShieldCheck className="w-4 h-4 text-primary" />
            <span className="text-xs uppercase tracking-widest font-display font-semibold">Zero-Knowledge Proof</span>
          </div>

          {!jobId ? (
            <div className="flex-1 flex flex-col justify-center">
              <div className="w-16 h-16 rounded-3xl bg-primary/10 grid place-items-center mx-auto mb-4">
                <ShieldCheck className="w-7 h-7 text-primary" />
              </div>
              <h3 className="font-display text-xl font-bold text-ink text-center">
                Private by default
              </h3>
              <p className="text-sm text-muted-foreground text-center mt-2 max-w-sm mx-auto leading-relaxed">
                Your payment is verified, never published. The network sees a proof — you keep the receipt.
              </p>
              <div className="mt-6 space-y-3">
                {[
                  "Sender, recipient, and amount stay private",
                  "The network sees a single proof byte string",
                  "You get a receipt only you can decrypt",
                ].map((t, i) => (
                  <div key={i} className="flex items-start gap-3 p-3 rounded-xl bg-muted/50">
                    <div className="w-5 h-5 rounded-full bg-primary/10 grid place-items-center shrink-0 mt-0.5">
                      <Check className="w-3 h-3 text-primary" />
                    </div>
                    <p className="text-sm text-muted-foreground">{t}</p>
                  </div>
                ))}
              </div>
            </div>
          ) : (
            <div className="mt-4 flex-1">
              <div className="flex items-center justify-between">
                <p className="font-display text-xl font-bold text-ink">Proof Job</p>
                <span className="text-[10px] text-muted-foreground font-mono">{jobId.slice(0, 16)}</span>
              </div>

              <div className="mt-6 space-y-4">
                {STEPS.map((step, idx) => {
                  const currentIdx = STEPS.findIndex((s) => s.key === (status?.status ?? "queued"));
                  const done = idx <= currentIdx;
                  const active = idx === currentIdx;

                  return (
                    <div key={step.key} className="flex items-start gap-4">
                      <div className="flex flex-col items-center">
                        <div
                          className={
                            "w-10 h-10 rounded-2xl grid place-items-center text-sm font-bold transition-all duration-500 " +
                            (done
                              ? "bg-primary text-primary-foreground shadow-soft"
                              : "bg-muted text-muted-foreground")
                          }
                        >
                          {done ? (
                            active && status?.status !== "verified" ? (
                              <Loader2 className="w-4 h-4 animate-spin" />
                            ) : (
                              <Check className="w-4 h-4" />
                            )
                          ) : (
                            idx + 1
                          )}
                        </div>
                        {idx < STEPS.length - 1 && (
                          <div className={
                            "w-0.5 h-8 mt-1 rounded-full transition-all duration-500 " +
                            (done ? "bg-primary" : "bg-muted")
                          } />
                        )}
                      </div>
                      <div className="pt-2">
                        <p className={"font-display font-semibold text-sm " + (done ? "text-ink" : "text-muted-foreground")}>
                          {step.label}
                        </p>
                        <p className="text-xs text-muted-foreground mt-0.5">{step.desc}</p>
                      </div>
                    </div>
                  );
                })}
              </div>

              {/* Progress bar */}
              <div className="mt-6">
                <div className="flex items-center justify-between text-xs mb-2">
                  <span className="text-muted-foreground font-display font-medium">Progress</span>
                  <span className="text-ink font-display font-semibold">
                    {status?.status === "verified" ? 100 : status?.progress ?? 5}%
                  </span>
                </div>
                <div className="h-2.5 rounded-full bg-muted overflow-hidden">
                  <div
                    className={
                      "h-full rounded-full transition-all duration-700 " +
                      (status?.status === "verified"
                        ? "bg-success"
                        : "bg-primary")
                    }
                    style={{ width: `${status?.status === "verified" ? 100 : status?.progress ?? 5}%` }}
                  />
                </div>
              </div>

              {status?.status === "verified" && (
                <div className="mt-4 p-4 rounded-2xl bg-success/10 border border-success/20 animate-scale-in">
                  <div className="flex items-center gap-2">
                    <Check className="w-5 h-5 text-success" />
                    <p className="font-display font-semibold text-success">Transaction Verified</p>
                  </div>
                  <p className="text-xs text-muted-foreground mt-1">
                    ZK proof confirmed. Funds are now private and settled on-chain.
                  </p>
                </div>
              )}
            </div>
          )}
        </aside>
      </div>
    </AppLayout>
  );
}
