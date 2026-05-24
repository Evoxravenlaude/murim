import { createFileRoute } from "@tanstack/react-router";
import { useMutation } from "@tanstack/react-query";
import { useState } from "react";
import { useServerFn } from "@tanstack/react-start";
import {
  Upload,
  Image,
  FileText,
  Palette,
  DollarSign,
  Check,
  Plus,
  Trash2,
  Sparkles,
  ArrowRight,
  ArrowLeft,
  Loader2,
  Music,
  Film,
  Box,
  ExternalLink,
} from "lucide-react";
import { AppLayout } from "@/components/nautica/AppLayout";
import { submitMint } from "@/lib/nautica.functions";

export const Route = createFileRoute("/create")(
  {
    head: () => ({
      meta: [
        { title: "Create NFT — Nautica" },
        { name: "description", content: "Create and mint NFTs on Nautica. Upload art, set traits, configure royalties, and deploy on-chain." },
        { property: "og:title", content: "Create NFT — Nautica" },
      ],
    }),
    component: CreatePage,
  },
);

type Trait = { key: string; value: string };

const CATEGORIES = ["Generative", "Photography", "Painting", "3D", "Music", "PFP"] as const;
const CHAINS = [
  { id: "ethereum", name: "Ethereum", icon: "⟠" },
  { id: "solana", name: "Solana", icon: "◎" },
  { id: "base", name: "Base", icon: "🔵" },
  { id: "polygon", name: "Polygon", icon: "⬡" },
] as const;

const STEPS = [
  { icon: Upload, label: "Upload" },
  { icon: FileText, label: "Details" },
  { icon: Palette, label: "Traits" },
  { icon: DollarSign, label: "Pricing" },
  { icon: Check, label: "Review" },
];

function CreatePage() {
  const [step, setStep] = useState(0);
  const [file, setFile] = useState<File | null>(null);
  const [preview, setPreview] = useState<string | null>(null);
  const [title, setTitle] = useState("");
  const [description, setDescription] = useState("");
  const [category, setCategory] = useState<string>("");
  const [chain, setChain] = useState("ethereum");
  const [traits, setTraits] = useState<Trait[]>([{ key: "", value: "" }]);
  const [supply, setSupply] = useState("1");
  const [price, setPrice] = useState("0.1");
  const [currency, setCurrency] = useState("ETH");
  const [royalty, setRoyalty] = useState("7.5");
  const [listingType, setListingType] = useState<"fixed" | "auction" | "open">("fixed");

  const mintFn = useServerFn(submitMint);
  const mintMutation = useMutation({
    mutationFn: () =>
      mintFn({
        data: {
          title,
          description,
          category,
          traits: traits.filter((t) => t.key && t.value),
          supply: Number(supply),
          price: Number(price),
          currency,
          royalty: Number(royalty),
        },
      }),
  });

  function handleFileChange(e: React.ChangeEvent<HTMLInputElement>) {
    const f = e.target.files?.[0];
    if (f) {
      setFile(f);
      const reader = new FileReader();
      reader.onloadend = () => setPreview(reader.result as string);
      reader.readAsDataURL(f);
    }
  }

  function addTrait() {
    setTraits([...traits, { key: "", value: "" }]);
  }

  function removeTrait(idx: number) {
    setTraits(traits.filter((_, i) => i !== idx));
  }

  function updateTrait(idx: number, field: "key" | "value", val: string) {
    setTraits(traits.map((t, i) => (i === idx ? { ...t, [field]: val } : t)));
  }

  const canNext =
    step === 0 ? !!file :
    step === 1 ? !!title && !!category :
    step === 2 ? true :
    step === 3 ? !!price :
    true;

  return (
    <AppLayout>
      <header className="animate-fade-down">
        <p className="text-xs text-primary font-display font-semibold uppercase tracking-widest">Studio</p>
        <h1 className="font-display text-4xl md:text-5xl font-bold text-ink mt-1">Create NFT</h1>
      </header>

      {/* Stepper */}
      <div className="mt-6 flex items-center justify-between gap-1 animate-fade-up">
        {STEPS.map((s, i) => (
          <button
            key={s.label}
            onClick={() => i < step && setStep(i)}
            className="flex-1 group"
          >
            <div className="flex items-center gap-2 justify-center mb-2">
              <div className={
                "w-9 h-9 rounded-xl grid place-items-center transition-all duration-300 text-xs font-bold " +
                (i < step
                  ? "bg-primary text-primary-foreground"
                  : i === step
                  ? "bg-primary/10 text-primary border border-primary/20"
                  : "bg-muted text-muted-foreground")
              }>
                {i < step ? <Check className="w-4 h-4" /> : <s.icon className="w-4 h-4" />}
              </div>
            </div>
            <div className={
              "h-1 rounded-full transition-all duration-500 " +
              (i < step ? "bg-primary" : i === step ? "bg-primary/30" : "bg-muted")
            } />
            <p className={
              "text-[10px] font-display font-semibold mt-1.5 text-center " +
              (i <= step ? "text-ink" : "text-muted-foreground")
            }>{s.label}</p>
          </button>
        ))}
      </div>

      <div className="mt-8 grid lg:grid-cols-[1.2fr_0.8fr] gap-6">
        {/* Main content */}
        <div className="nautica-glass p-6 md:p-8 animate-fade-up" style={{ animationDelay: "100ms" }}>
          {/* Step 0: Upload */}
          {step === 0 && (
            <div className="animate-fade-in">
              <h2 className="font-display text-xl font-bold text-ink">Upload your artwork</h2>
              <p className="text-sm text-muted-foreground mt-1">
                Supports JPG, PNG, GIF, SVG, MP4, WEBM, MP3, WAV, GLB, GLTF. Max 100MB.
              </p>

              <label className="mt-6 flex flex-col items-center justify-center w-full aspect-[4/3] rounded-2xl border-2 border-dashed border-border hover:border-primary/40 bg-muted/30 cursor-pointer transition-all duration-300 group">
                <input type="file" className="hidden" onChange={handleFileChange} accept="image/*,video/*,audio/*,.glb,.gltf" />
                {preview ? (
                  <img src={preview} alt="Preview" className="w-full h-full object-cover rounded-2xl" />
                ) : (
                  <div className="text-center p-8">
                    <div className="w-16 h-16 rounded-3xl bg-primary/10 grid place-items-center mx-auto mb-4 group-hover:bg-primary/20 transition-colors">
                      <Upload className="w-7 h-7 text-primary" />
                    </div>
                    <p className="font-display font-semibold text-ink">
                      Drop file or click to upload
                    </p>
                    <div className="flex items-center justify-center gap-3 mt-3">
                      {[
                        { icon: Image, label: "Image" },
                        { icon: Film, label: "Video" },
                        { icon: Music, label: "Audio" },
                        { icon: Box, label: "3D" },
                      ].map((t) => (
                        <span key={t.label} className="flex items-center gap-1 text-xs text-muted-foreground">
                          <t.icon className="w-3 h-3" /> {t.label}
                        </span>
                      ))}
                    </div>
                  </div>
                )}
              </label>

              {file && (
                <div className="mt-4 flex items-center gap-3 p-3 rounded-xl bg-muted/50 border border-border/50">
                  <Image className="w-5 h-5 text-primary shrink-0" />
                  <div className="flex-1 min-w-0">
                    <p className="text-sm text-ink font-medium truncate">{file.name}</p>
                    <p className="text-[11px] text-muted-foreground">{(file.size / 1024 / 1024).toFixed(2)} MB</p>
                  </div>
                  <button onClick={() => { setFile(null); setPreview(null); }} className="btn-icon w-8 h-8">
                    <Trash2 className="w-3.5 h-3.5" />
                  </button>
                </div>
              )}
            </div>
          )}

          {/* Step 1: Details */}
          {step === 1 && (
            <div className="animate-fade-in space-y-5">
              <h2 className="font-display text-xl font-bold text-ink">Details</h2>

              <label className="block">
                <span className="text-sm text-muted-foreground font-display font-medium">Title *</span>
                <input value={title} onChange={(e) => setTitle(e.target.value)} placeholder="e.g. Tideglass #04" className="nautica-input mt-2" />
              </label>

              <label className="block">
                <span className="text-sm text-muted-foreground font-display font-medium">Description</span>
                <textarea
                  value={description}
                  onChange={(e) => setDescription(e.target.value)}
                  placeholder="Tell the story behind your artwork…"
                  rows={4}
                  className="nautica-input mt-2 resize-none"
                />
              </label>

              <div>
                <span className="text-sm text-muted-foreground font-display font-medium">Category *</span>
                <div className="mt-2 grid grid-cols-3 gap-2">
                  {CATEGORIES.map((c) => (
                    <button
                      key={c}
                      onClick={() => setCategory(c)}
                      className={
                        "py-3 rounded-xl text-sm font-display font-semibold transition-all " +
                        (category === c
                          ? "bg-primary text-primary-foreground shadow-soft"
                          : "bg-muted text-muted-foreground hover:bg-secondary hover:text-foreground")
                      }
                    >
                      {c}
                    </button>
                  ))}
                </div>
              </div>

              <div>
                <span className="text-sm text-muted-foreground font-display font-medium">Blockchain</span>
                <div className="mt-2 grid grid-cols-2 gap-2">
                  {CHAINS.map((c) => (
                    <button
                      key={c.id}
                      onClick={() => setChain(c.id)}
                      className={
                        "flex items-center gap-2 p-3 rounded-xl text-sm font-display font-medium transition-all " +
                        (chain === c.id
                          ? "bg-primary/10 text-primary border border-primary/20"
                          : "bg-muted text-muted-foreground hover:bg-secondary border border-transparent")
                      }
                    >
                      <span className="text-lg">{c.icon}</span>
                      {c.name}
                    </button>
                  ))}
                </div>
              </div>
            </div>
          )}

          {/* Step 2: Traits */}
          {step === 2 && (
            <div className="animate-fade-in">
              <h2 className="font-display text-xl font-bold text-ink">Traits & Properties</h2>
              <p className="text-sm text-muted-foreground mt-1">
                Add traits to make your NFT discoverable and unique. These appear on the detail page.
              </p>

              <div className="mt-6 space-y-3">
                {traits.map((t, i) => (
                  <div key={i} className="flex gap-2 items-center">
                    <input
                      value={t.key}
                      onChange={(e) => updateTrait(i, "key", e.target.value)}
                      placeholder="Trait name"
                      className="nautica-input flex-1"
                    />
                    <input
                      value={t.value}
                      onChange={(e) => updateTrait(i, "value", e.target.value)}
                      placeholder="Value"
                      className="nautica-input flex-1"
                    />
                    {traits.length > 1 && (
                      <button onClick={() => removeTrait(i)} className="btn-icon w-10 h-10 shrink-0 text-destructive">
                        <Trash2 className="w-4 h-4" />
                      </button>
                    )}
                  </div>
                ))}
              </div>

              <button onClick={addTrait} className="mt-4 btn-secondary text-xs w-full">
                <Plus className="w-4 h-4" /> Add Trait
              </button>
            </div>
          )}

          {/* Step 3: Pricing */}
          {step === 3 && (
            <div className="animate-fade-in space-y-5">
              <h2 className="font-display text-xl font-bold text-ink">Pricing & Supply</h2>

              <div>
                <span className="text-sm text-muted-foreground font-display font-medium">Listing Type</span>
                <div className="mt-2 grid grid-cols-3 gap-2">
                  {([
                    { id: "fixed", label: "Fixed Price", desc: "Set price" },
                    { id: "auction", label: "Auction", desc: "Timed bidding" },
                    { id: "open", label: "Open Edition", desc: "Unlimited" },
                  ] as const).map((lt) => (
                    <button
                      key={lt.id}
                      onClick={() => setListingType(lt.id)}
                      className={
                        "p-3 rounded-xl text-center transition-all " +
                        (listingType === lt.id
                          ? "bg-primary/10 text-primary border border-primary/20"
                          : "bg-muted text-muted-foreground border border-transparent hover:bg-secondary")
                      }
                    >
                      <p className="font-display font-semibold text-sm">{lt.label}</p>
                      <p className="text-[10px] mt-0.5 opacity-70">{lt.desc}</p>
                    </button>
                  ))}
                </div>
              </div>

              <div className="grid grid-cols-2 gap-4">
                <label className="block">
                  <span className="text-sm text-muted-foreground font-display font-medium">Price *</span>
                  <div className="relative mt-2">
                    <input value={price} onChange={(e) => setPrice(e.target.value)} inputMode="decimal" className="nautica-input pr-16" />
                    <select
                      value={currency}
                      onChange={(e) => setCurrency(e.target.value)}
                      className="absolute right-2 top-1/2 -translate-y-1/2 bg-muted rounded-lg px-2 py-1 text-xs font-display font-semibold text-ink outline-none cursor-pointer"
                    >
                      <option>ETH</option>
                      <option>USDC</option>
                      <option>SOL</option>
                    </select>
                  </div>
                </label>
                <label className="block">
                  <span className="text-sm text-muted-foreground font-display font-medium">Supply</span>
                  <input value={supply} onChange={(e) => setSupply(e.target.value)} inputMode="numeric" className="nautica-input mt-2" />
                </label>
              </div>

              <label className="block">
                <span className="text-sm text-muted-foreground font-display font-medium">
                  Creator Royalty: <span className="text-primary font-bold">{royalty}%</span>
                </span>
                <input
                  type="range"
                  min="0"
                  max="15"
                  step="0.5"
                  value={royalty}
                  onChange={(e) => setRoyalty(e.target.value)}
                  className="w-full mt-3 accent-primary"
                />
                <div className="flex justify-between text-[10px] text-muted-foreground mt-1">
                  <span>0%</span>
                  <span>15%</span>
                </div>
              </label>
            </div>
          )}

          {/* Step 4: Review */}
          {step === 4 && (
            <div className="animate-fade-in space-y-4">
              <h2 className="font-display text-xl font-bold text-ink">Review & Mint</h2>

              {mintMutation.isSuccess ? (
                <div className="text-center py-8 animate-scale-in">
                  <div className="w-20 h-20 rounded-3xl bg-success/10 grid place-items-center mx-auto mb-4">
                    <Check className="w-9 h-9 text-success" />
                  </div>
                  <h3 className="font-display text-2xl font-bold text-ink">Successfully Minted! 🎉</h3>
                  <p className="text-sm text-muted-foreground mt-2">
                    Your NFT is now live on the blockchain.
                  </p>
                  <p className="text-xs text-muted-foreground font-mono mt-1">
                    Token: {mintMutation.data?.tokenId}
                  </p>
                  <div className="flex justify-center gap-3 mt-6">
                    <button className="btn-primary text-xs">
                      <ExternalLink className="w-4 h-4" /> View on Explorer
                    </button>
                    <button onClick={() => { setStep(0); setFile(null); setPreview(null); setTitle(""); mintMutation.reset(); }} className="btn-secondary text-xs">
                      Create Another
                    </button>
                  </div>
                </div>
              ) : (
                <>
                  <div className="space-y-3">
                    {[
                      { label: "Title", value: title },
                      { label: "Category", value: category },
                      { label: "Chain", value: CHAINS.find((c) => c.id === chain)?.name ?? chain },
                      { label: "Supply", value: supply },
                      { label: "Price", value: `${price} ${currency}` },
                      { label: "Royalty", value: `${royalty}%` },
                      { label: "Listing", value: listingType },
                      { label: "Traits", value: `${traits.filter((t) => t.key).length} properties` },
                    ].map((r) => (
                      <div key={r.label} className="flex items-center justify-between p-3 rounded-xl bg-muted/30 border border-border/50">
                        <span className="text-sm text-muted-foreground">{r.label}</span>
                        <span className="text-sm font-display font-semibold text-ink">{r.value}</span>
                      </div>
                    ))}
                  </div>

                  {/* Gas estimate */}
                  <div className="p-4 rounded-2xl bg-primary/5 border border-primary/10">
                    <div className="flex items-center justify-between">
                      <span className="text-sm text-muted-foreground">Estimated Gas</span>
                      <span className="font-display font-semibold text-ink">~0.003 ETH</span>
                    </div>
                    <p className="text-[10px] text-muted-foreground mt-1">≈ $11.54 at current prices</p>
                  </div>

                  <button
                    onClick={() => mintMutation.mutate()}
                    disabled={mintMutation.isPending}
                    className="w-full btn-cta py-4 mt-2"
                  >
                    {mintMutation.isPending ? (
                      <><Loader2 className="w-4 h-4 animate-spin" /> Minting…</>
                    ) : (
                      <><Sparkles className="w-4 h-4" /> Mint NFT</>
                    )}
                  </button>
                </>
              )}
            </div>
          )}

          {/* Navigation buttons */}
          {!(step === 4 && mintMutation.isSuccess) && (
            <div className="mt-8 flex gap-3">
              {step > 0 && (
                <button onClick={() => setStep(step - 1)} className="btn-secondary">
                  <ArrowLeft className="w-4 h-4" /> Back
                </button>
              )}
              {step < 4 && (
                <button
                  onClick={() => setStep(step + 1)}
                  disabled={!canNext}
                  className="btn-primary ml-auto"
                >
                  Next <ArrowRight className="w-4 h-4" />
                </button>
              )}
            </div>
          )}
        </div>

        {/* Live preview sidebar */}
        <div className="hidden lg:block">
          <div className="sticky top-8">
            <p className="text-xs text-muted-foreground font-display font-semibold uppercase tracking-widest mb-3">
              Live Preview
            </p>
            <div className="nautica-glass overflow-hidden animate-scale-in">
              {/* Preview image */}
              <div className="aspect-[4/5] w-full relative overflow-hidden bg-muted">
                {preview ? (
                  <img src={preview} alt="Preview" className="w-full h-full object-cover" />
                ) : (
                  <div className="absolute inset-0 grid place-items-center">
                    <div className="text-center">
                      <Image className="w-10 h-10 text-muted-foreground mx-auto mb-2" />
                      <p className="text-xs text-muted-foreground">Upload to preview</p>
                    </div>
                  </div>
                )}
                {category && (
                  <div className="absolute top-3 left-3">
                    <span className="px-2.5 py-1 rounded-full bg-black/30 backdrop-blur-xl text-white text-[10px] font-semibold uppercase tracking-wider">
                      {category}
                    </span>
                  </div>
                )}
              </div>

              {/* Preview info */}
              <div className="p-4">
                <p className="text-[11px] text-primary font-medium">You</p>
                <h3 className="font-display text-lg font-semibold text-ink truncate mt-0.5">
                  {title || "Untitled"}
                </h3>
                <div className="flex items-center justify-between mt-3">
                  <div>
                    <p className="text-[10px] text-muted-foreground uppercase tracking-wider">Price</p>
                    <p className="font-display font-bold text-ink">
                      {price || "—"} <span className="text-xs text-muted-foreground">{currency}</span>
                    </p>
                  </div>
                  <div className="text-right">
                    <p className="text-[10px] text-muted-foreground uppercase tracking-wider">Edition</p>
                    <p className="font-display font-bold text-ink">
                      {listingType === "open" ? "Open" : `1 of ${supply}`}
                    </p>
                  </div>
                </div>
              </div>
            </div>
          </div>
        </div>
      </div>
    </AppLayout>
  );
}
