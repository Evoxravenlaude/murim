import { useState } from "react";
import { Link } from "react-router-dom";
import { Grid3X3, List, Search, Tag, Send } from "lucide-react";
import { useAccount } from "wagmi";
import { useQuery } from "@tanstack/react-query";
import { fetchNFTsForOwner } from "@/lib/api";

export default function MyNFTs() {
  const [viewMode, setViewMode] = useState<"grid" | "list">("grid");
  const [selected, setSelected] = useState<string[]>([]);
  const [searchTerm, setSearchTerm] = useState("");
  const { address, isConnected } = useAccount();

  const { data: ownedNFTs, isLoading } = useQuery({
    queryKey: ["my-nfts", address],
    queryFn: () => fetchNFTsForOwner(address!),
    enabled: !!address,
    staleTime: 60_000,
  });

  const nfts = (ownedNFTs ?? []).filter(
    (n) => n.name.toLowerCase().includes(searchTerm.toLowerCase()) ||
           n.contractAddress.toLowerCase().includes(searchTerm.toLowerCase())
  );

  const toggleSelect = (id: string) => {
    setSelected((prev) =>
      prev.includes(id) ? prev.filter((i) => i !== id) : [...prev, id]
    );
  };

  if (!isConnected) {
    return (
      <div className="min-h-screen bg-void flex flex-col items-center justify-center px-4">
        <div className="w-16 h-16 rounded-full bg-cyan/10 flex items-center justify-center mb-6">
          <span className="font-heading text-2xl text-cyan">N</span>
        </div>
        <h1 className="font-heading text-xl text-text-primary">Connect Your Wallet</h1>
        <p className="mt-2 text-text-secondary text-sm text-center max-w-sm">
          Connect your wallet to view your NFT collection.
        </p>
      </div>
    );
  }

  if (isLoading) {
    return (
      <div className="min-h-screen bg-void flex flex-col items-center justify-center px-4">
        <div className="w-8 h-8 border-2 border-cyan border-t-transparent rounded-full animate-spin mb-4" />
        <p className="font-mono text-sm text-text-secondary">Loading your NFTs...</p>
      </div>
    );
  }

  if (!nfts.length && !searchTerm) {
    return (
      <div className="min-h-screen bg-void flex flex-col items-center justify-center px-4">
        <div className="w-16 h-16 rounded-full bg-white/5 flex items-center justify-center mb-6">
          <span className="font-heading text-2xl text-text-tertiary">0</span>
        </div>
        <h1 className="font-heading text-xl text-text-primary">No NFTs Yet</h1>
        <p className="mt-2 text-text-secondary text-sm text-center max-w-sm">
          Start building your collection by browsing the marketplace or creating your own NFTs.
        </p>
        <div className="mt-6 flex gap-3">
          <Link to="/nft" className="px-6 py-3 bg-cyan text-void font-mono text-sm tracking-wider font-medium">
            Browse Marketplace
          </Link>
          <Link to="/create-nft" className="px-6 py-3 border border-white/10 text-text-primary font-mono text-sm tracking-wider hover:border-cyan/30">
            Create NFT
          </Link>
        </div>
      </div>
    );
  }

  return (
    <div className="min-h-screen bg-void px-4 py-6">
      <div className="max-w-5xl mx-auto">
        {/* Header */}
        <div className="flex flex-col sm:flex-row sm:items-center justify-between gap-4 mb-8">
          <div>
            <h1 className="font-heading text-2xl text-text-primary">My Collection</h1>
            <p className="mt-1 text-text-secondary text-sm">{nfts.length} NFTs</p>
          </div>
          <div className="flex items-center gap-3">
            <div className="relative">
              <Search size={16} className="absolute left-3 top-1/2 -translate-y-1/2 text-text-tertiary" />
              <input
                type="text"
                value={searchTerm}
                onChange={(e) => setSearchTerm(e.target.value)}
                className="bg-obsidian border border-white/10 pl-10 pr-4 py-2 font-mono text-sm text-text-primary placeholder:text-text-tertiary focus:outline-none focus:border-cyan"
                placeholder="Search..."
              />
            </div>
            <div className="flex border border-white/10">
              <button
                onClick={() => setViewMode("grid")}
                className={`p-2 ${viewMode === "grid" ? "text-cyan" : "text-text-tertiary"}`}
              >
                <Grid3X3 size={16} />
              </button>
              <button
                onClick={() => setViewMode("list")}
                className={`p-2 ${viewMode === "list" ? "text-cyan" : "text-text-tertiary"}`}
              >
                <List size={16} />
              </button>
            </div>
          </div>
        </div>

        {/* Stats */}
        <div className="grid grid-cols-3 gap-4 mb-8">
          {[
            { label: "Total NFTs", value: nfts.length.toString() },
            { label: "Collections", value: new Set(nfts.map(n => n.contractAddress)).size.toString() },
            { label: "Unique Contracts", value: new Set(nfts.map(n => n.contractAddress)).size.toString() },
          ].map((stat) => (
            <div key={stat.label} className="p-4 bg-obsidian border border-white/5">
              <p className="font-mono text-[10px] text-text-secondary">{stat.label}</p>
              <p className="font-heading text-xl text-text-primary mt-1">{stat.value}</p>
            </div>
          ))}
        </div>

        {/* Bulk Actions */}
        {selected.length > 0 && (
          <div className="flex items-center gap-3 mb-4 p-3 bg-cyan/5 border border-cyan/20">
            <span className="font-mono text-xs text-cyan">{selected.length} selected</span>
            <div className="flex gap-2 ml-auto">
              <button className="flex items-center gap-1 px-3 py-1 bg-obsidian border border-white/10 font-mono text-xs text-text-primary hover:border-cyan/30">
                <Tag size={12} /> List
              </button>
              <button className="flex items-center gap-1 px-3 py-1 bg-obsidian border border-white/10 font-mono text-xs text-text-primary hover:border-cyan/30">
                <Send size={12} /> Transfer
              </button>
            </div>
          </div>
        )}

        {/* NFT Grid */}
        <div className={`grid gap-4 ${viewMode === "grid" ? "grid-cols-2 sm:grid-cols-3 lg:grid-cols-4" : "grid-cols-1"}`}>
          {nfts.map((nft) => {
            const nftKey = `${nft.contractAddress}-${nft.tokenId}`;
            return (
              <div
                key={nftKey}
                className={`group bg-obsidian border transition-all cursor-pointer ${
                  selected.includes(nftKey) ? "border-cyan/50" : "border-white/5 hover:border-cyan/30"
                }`}
                onClick={() => toggleSelect(nftKey)}
              >
                <div className={`overflow-hidden relative bg-white/5 ${viewMode === "grid" ? "aspect-square" : "aspect-[2/1]"}`}>
                  {nft.image ? (
                    <img src={nft.image} alt={nft.name} className="w-full h-full object-cover" />
                  ) : (
                    <div className="w-full h-full flex items-center justify-center">
                      <span className="font-heading text-2xl text-text-tertiary">#{nft.tokenId}</span>
                    </div>
                  )}
                  {selected.includes(nftKey) && (
                    <div className="absolute top-2 left-2 w-5 h-5 bg-cyan flex items-center justify-center">
                      <svg width="12" height="12" viewBox="0 0 12 12" fill="none"><path d="M2 6L5 9L10 3" stroke="#050505" strokeWidth="2"/></svg>
                    </div>
                  )}
                </div>
                <div className="p-3">
                  <p className="font-mono text-[10px] text-text-secondary">{nft.contractAddress}</p>
                  <p className="font-body text-sm text-text-primary">{nft.name}</p>
                </div>
              </div>
            );
          })}
        </div>
      </div>
    </div>
  );
}
