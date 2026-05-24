import { useState, useRef } from "react";
import { Link } from "react-router-dom";
import { ChevronLeft, Upload, Plus, X, Check } from "lucide-react";

export default function CreateNFT() {
  const [step, setStep] = useState(1);
  const [image, setImage] = useState<string | null>(null);
  const [name, setName] = useState("");
  const [description, setDescription] = useState("");
  const [properties, setProperties] = useState<{ key: string; value: string }[]>([]);
  const [minting, setMinting] = useState(false);
  const [minted, setMinted] = useState(false);
  const fileRef = useRef<HTMLInputElement>(null);

  const handleImageUpload = (e: React.ChangeEvent<HTMLInputElement>) => {
    const file = e.target.files?.[0];
    if (file) {
      const reader = new FileReader();
      reader.onloadend = () => setImage(reader.result as string);
      reader.readAsDataURL(file);
    }
  };

  const addProperty = () => {
    setProperties([...properties, { key: "", value: "" }]);
  };

  const updateProperty = (i: number, field: "key" | "value", val: string) => {
    const next = [...properties];
    next[i][field] = val;
    setProperties(next);
  };

  const removeProperty = (i: number) => {
    setProperties(properties.filter((_, j) => j !== i));
  };

  const handleMint = () => {
    setMinting(true);
    setTimeout(() => {
      setMinting(false);
      setMinted(true);
    }, 2500);
  };

  if (minted) {
    return (
      <div className="min-h-screen bg-void flex flex-col items-center justify-center px-4">
        <div className="w-20 h-20 rounded-full bg-cyan/10 flex items-center justify-center mb-6 animate-pulse-glow">
          <Check size={32} className="text-cyan" />
        </div>
        <h1 className="font-heading text-2xl text-text-primary">NFT Minted!</h1>
        <p className="mt-2 text-text-secondary text-sm text-center">
          Your NFT has been successfully created and minted on-chain.
        </p>
        {image && (
          <img src={image} alt="Minted" className="w-32 h-32 object-cover mt-6 border border-cyan/30" />
        )}
        <div className="mt-6 flex gap-3">
          <Link to="/my-nfts" className="px-6 py-3 bg-cyan text-void font-mono text-sm tracking-wider font-medium">
            View My NFTs
          </Link>
          <Link to="/nft" className="px-6 py-3 border border-white/10 text-text-primary font-mono text-sm tracking-wider hover:border-cyan/30">
            Browse Marketplace
          </Link>
        </div>
      </div>
    );
  }

  return (
    <div className="min-h-screen bg-void px-4 py-6">
      <div className="max-w-lg mx-auto">
        {/* Header */}
        <div className="flex items-center gap-3 mb-8">
          <Link to="/nft" className="text-text-secondary hover:text-text-primary">
            <ChevronLeft size={20} />
          </Link>
          <h1 className="font-heading text-xl text-text-primary">Create NFT</h1>
        </div>

        {/* Step Indicator */}
        <div className="flex items-center gap-2 mb-8">
          {[1, 2, 3, 4].map((s) => (
            <div key={s} className="flex items-center gap-2">
              <div
                className={`w-8 h-8 rounded-full flex items-center justify-center font-mono text-xs ${
                  s <= step ? "bg-cyan text-void" : "bg-white/5 text-text-tertiary"
                }`}
              >
                {s < step ? <Check size={14} /> : s}
              </div>
              {s < 4 && <div className={`w-8 h-[2px] ${s < step ? "bg-cyan" : "bg-white/10"}`} />}
            </div>
          ))}
        </div>

        {/* Step 1: Upload */}
        {step === 1 && (
          <div>
            <h2 className="font-heading text-lg text-text-primary mb-4">Upload Image</h2>
            <p className="text-text-secondary text-sm mb-6">
              Drag and drop or click to upload your NFT artwork.
            </p>
            <button
              onClick={() => fileRef.current?.click()}
              className="w-full aspect-square bg-obsidian border-2 border-dashed border-white/10 hover:border-cyan/30 transition-colors flex flex-col items-center justify-center gap-3"
            >
              {image ? (
                <img src={image} alt="Preview" className="w-full h-full object-cover" />
              ) : (
                <>
                  <Upload size={32} className="text-text-tertiary" />
                  <span className="font-mono text-xs text-text-secondary">Click to upload</span>
                  <span className="font-mono text-[10px] text-text-tertiary">JPG, PNG, GIF up to 50MB</span>
                </>
              )}
            </button>
            <input
              ref={fileRef}
              type="file"
              accept="image/*"
              onChange={handleImageUpload}
              className="hidden"
            />
            <button
              onClick={() => image && setStep(2)}
              disabled={!image}
              className="mt-6 w-full py-4 bg-cyan text-void font-mono text-sm tracking-wider font-medium hover:bg-opacity-90 transition-all disabled:opacity-30"
            >
              Continue
            </button>
          </div>
        )}

        {/* Step 2: Details */}
        {step === 2 && (
          <div className="space-y-6">
            <h2 className="font-heading text-lg text-text-primary">NFT Details</h2>

            <div>
              <label className="block font-mono text-xs text-text-secondary mb-2">Name</label>
              <input
                type="text"
                value={name}
                onChange={(e) => setName(e.target.value)}
                className="w-full bg-obsidian border border-white/10 px-4 py-3 text-text-primary font-body focus:outline-none focus:border-cyan"
                placeholder="My Awesome NFT"
              />
            </div>

            <div>
              <label className="block font-mono text-xs text-text-secondary mb-2">Description</label>
              <textarea
                value={description}
                onChange={(e) => setDescription(e.target.value)}
                className="w-full h-32 bg-obsidian border border-white/10 px-4 py-3 text-text-primary font-body focus:outline-none focus:border-cyan resize-none"
                placeholder="Describe your NFT..."
              />
            </div>

            {/* Properties */}
            <div>
              <div className="flex items-center justify-between mb-2">
                <label className="font-mono text-xs text-text-secondary">Properties</label>
                <button onClick={addProperty} className="text-cyan font-mono text-xs hover:opacity-80">
                  <Plus size={12} className="inline" /> Add
                </button>
              </div>
              {properties.map((prop, i) => (
                <div key={i} className="flex gap-2 mb-2">
                  <input
                    type="text"
                    value={prop.key}
                    onChange={(e) => updateProperty(i, "key", e.target.value)}
                    className="flex-1 bg-obsidian border border-white/10 px-3 py-2 text-text-primary font-mono text-xs focus:outline-none focus:border-cyan"
                    placeholder="Trait"
                  />
                  <input
                    type="text"
                    value={prop.value}
                    onChange={(e) => updateProperty(i, "value", e.target.value)}
                    className="flex-1 bg-obsidian border border-white/10 px-3 py-2 text-text-primary font-mono text-xs focus:outline-none focus:border-cyan"
                    placeholder="Value"
                  />
                  <button onClick={() => removeProperty(i)} className="text-text-tertiary hover:text-red">
                    <X size={14} />
                  </button>
                </div>
              ))}
            </div>

            <div className="flex gap-3">
              <button onClick={() => setStep(1)} className="px-6 py-3 border border-white/10 text-text-secondary font-mono text-sm">
                Back
              </button>
              <button
                onClick={() => name && setStep(3)}
                disabled={!name}
                className="flex-1 py-4 bg-cyan text-void font-mono text-sm tracking-wider font-medium hover:bg-opacity-90 transition-all disabled:opacity-30"
              >
                Continue
              </button>
            </div>
          </div>
        )}

        {/* Step 3: Preview */}
        {step === 3 && (
          <div>
            <h2 className="font-heading text-lg text-text-primary mb-4">Preview</h2>
            <div className="bg-obsidian border border-white/5 p-4 mb-6">
              {image && (
                <img src={image} alt="Preview" className="w-full aspect-square object-cover mb-4" />
              )}
              <h3 className="font-heading text-lg text-text-primary">{name}</h3>
              <p className="text-text-secondary text-sm mt-1">{description}</p>
              {properties.length > 0 && (
                <div className="flex flex-wrap gap-2 mt-4">
                  {properties.map((p, i) => (
                    <span key={i} className="px-2 py-1 bg-white/5 font-mono text-[10px] text-text-secondary">
                      {p.key}: {p.value}
                    </span>
                  ))}
                </div>
              )}
            </div>
            <div className="flex gap-3">
              <button onClick={() => setStep(2)} className="px-6 py-3 border border-white/10 text-text-secondary font-mono text-sm">
                Back
              </button>
              <button onClick={() => setStep(4)} className="flex-1 py-4 bg-cyan text-void font-mono text-sm tracking-wider font-medium">
                Continue to Mint
              </button>
            </div>
          </div>
        )}

        {/* Step 4: Mint */}
        {step === 4 && (
          <div>
            <h2 className="font-heading text-lg text-text-primary mb-4">Mint NFT</h2>
            <div className="p-4 bg-obsidian border border-white/5 mb-6 space-y-3">
              <div className="flex items-center justify-between">
                <span className="font-mono text-xs text-text-secondary">Network</span>
                <span className="font-mono text-xs text-cyan">zkSync Era</span>
              </div>
              <div className="flex items-center justify-between">
                <span className="font-mono text-xs text-text-secondary">Mint Fee</span>
                <span className="font-mono text-xs text-text-primary">~$0.25</span>
              </div>
              <div className="flex items-center justify-between">
                <span className="font-mono text-xs text-text-secondary">Royalty</span>
                <span className="font-mono text-xs text-text-primary">5%</span>
              </div>
            </div>

            {minting ? (
              <div className="text-center py-8">
                <div className="w-12 h-12 border-2 border-cyan border-t-transparent rounded-full animate-spin mx-auto mb-4" />
                <p className="font-mono text-sm text-text-primary">Minting your NFT...</p>
                <p className="font-mono text-xs text-text-secondary mt-1">Confirm in your wallet</p>
              </div>
            ) : (
              <div className="flex gap-3">
                <button onClick={() => setStep(3)} className="px-6 py-3 border border-white/10 text-text-secondary font-mono text-sm">
                  Back
                </button>
                <button
                  onClick={handleMint}
                  className="flex-1 py-4 bg-cyan text-void font-mono text-sm tracking-wider font-medium hover:bg-opacity-90 transition-all"
                >
                  Mint NFT
                </button>
              </div>
            )}
          </div>
        )}
      </div>
    </div>
  );
}
