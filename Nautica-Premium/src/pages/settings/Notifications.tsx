import { useState } from "react";
import { Link } from "react-router-dom";
import { ChevronLeft, Check, Bell, TrendingUp, Info } from "lucide-react";

const NOTIFICATIONS = [
  { id: 1, type: "transaction", title: "Transaction Confirmed", message: "Your swap of 0.5 ETH for NAUT has been confirmed", time: "2 min ago", read: false },
  { id: 2, type: "transaction", title: "Transaction Failed", message: "Insufficient gas for transaction 0x7a...3f2a", time: "15 min ago", read: false },
  { id: 3, type: "price", title: "ETH Price Alert", message: "ETH reached your target price of $2,800", time: "1 hr ago", read: true },
  { id: 4, type: "system", title: "System Update", message: "Nautica v2.1.0 is now available", time: "3 hr ago", read: true },
  { id: 5, type: "transaction", title: "NFT Sale", message: "Node #2847 sold for 2.4 ETH", time: "5 hr ago", read: true },
  { id: 6, type: "price", title: "NAUT Price Alert", message: "NAUT is up 12% in the last 24 hours", time: "1 day ago", read: true },
];

export default function Notifications() {
  const [filter, setFilter] = useState<"all" | "transactions" | "price" | "system">("all");
  const [notifs, setNotifs] = useState(NOTIFICATIONS);

  const filtered = notifs.filter((n) => {
    if (filter === "transactions") return n.type === "transaction";
    if (filter === "price") return n.type === "price";
    if (filter === "system") return n.type === "system";
    return true;
  });

  const markAllRead = () => {
    setNotifs(notifs.map((n) => ({ ...n, read: true })));
  };

  const clearAll = () => {
    setNotifs([]);
  };

  const getIcon = (type: string) => {
    switch (type) {
      case "transaction":
        return { icon: Check, color: "text-cyan", bg: "bg-cyan/10" };
      case "price":
        return { icon: TrendingUp, color: "text-amber", bg: "bg-amber/10" };
      case "system":
        return { icon: Info, color: "text-blue", bg: "bg-blue/10" };
      default:
        return { icon: Bell, color: "text-text-secondary", bg: "bg-white/5" };
    }
  };

  return (
    <div className="min-h-screen bg-void px-4 py-6">
      <div className="max-w-lg mx-auto">
        {/* Header */}
        <div className="flex items-center justify-between mb-8">
          <div className="flex items-center gap-3">
            <Link to="/" className="text-text-secondary hover:text-text-primary">
              <ChevronLeft size={20} />
            </Link>
            <h1 className="font-heading text-xl text-text-primary">Notifications</h1>
          </div>
          <div className="flex gap-2">
            <button onClick={markAllRead} className="font-mono text-xs text-cyan hover:opacity-80">
              Mark All Read
            </button>
            <button onClick={clearAll} className="font-mono text-xs text-text-secondary hover:text-red">
              Clear All
            </button>
          </div>
        </div>

        {/* Filter Tabs */}
        <div className="flex gap-4 mb-6 border-b border-white/5">
          {(["all", "transactions", "price", "system"] as const).map((f) => (
            <button
              key={f}
              onClick={() => setFilter(f)}
              className={`pb-3 font-mono text-xs capitalize transition-colors ${
                filter === f ? "text-cyan border-b-2 border-cyan" : "text-text-secondary hover:text-text-primary"
              }`}
            >
              {f}
              {f === "all" && notifs.some((n) => !n.read) && (
                <span className="ml-1 w-2 h-2 bg-amber rounded-full inline-block" />
              )}
            </button>
          ))}
        </div>

        {/* Notifications List */}
        {filtered.length === 0 ? (
          <div className="text-center py-16">
            <Bell size={32} className="text-text-tertiary mx-auto mb-4" />
            <p className="text-text-secondary text-sm">No notifications</p>
          </div>
        ) : (
          <div className="space-y-1">
            {filtered.map((n) => {
              const { icon: Icon, color, bg } = getIcon(n.type);
              return (
                <div
                  key={n.id}
                  className={`flex items-start gap-3 p-4 transition-colors ${
                    n.read ? "bg-obsidian/30" : "bg-obsidian/70"
                  }`}
                >
                  <div className={`w-8 h-8 rounded-full ${bg} flex items-center justify-center shrink-0 mt-0.5`}>
                    <Icon size={14} className={color} />
                  </div>
                  <div className="flex-1 min-w-0">
                    <div className="flex items-center gap-2">
                      <p className="font-body text-sm text-text-primary">{n.title}</p>
                      {!n.read && <span className="w-2 h-2 bg-amber rounded-full shrink-0" />}
                    </div>
                    <p className="text-text-secondary text-sm mt-1">{n.message}</p>
                    <p className="font-mono text-[10px] text-text-tertiary mt-2">{n.time}</p>
                  </div>
                </div>
              );
            })}
          </div>
        )}
      </div>
    </div>
  );
}
