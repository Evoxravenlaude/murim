import { useState } from "react";
import { Link } from "react-router-dom";
import {
  ChevronRight, Moon, Sun, Monitor, Globe, Lock, Clock,
  Bell, Trash2, LogOut, Shield,
} from "lucide-react";

export default function Settings() {
  const [theme, setTheme] = useState<"dark" | "light" | "system">("dark");
  const [currency, setCurrency] = useState("USD");
  const [biometrics, setBiometrics] = useState(true);
  const [notifications, setNotifications] = useState(true);
  const [showLogoutConfirm, setShowLogoutConfirm] = useState(false);

  return (
    <div className="min-h-screen bg-void px-4 py-6">
      <div className="max-w-lg mx-auto">
        <h1 className="font-heading text-2xl text-text-primary mb-8">Settings</h1>

        {/* General */}
        <div className="mb-8">
          <h2 className="font-mono text-xs text-text-secondary uppercase tracking-wider mb-4">
            General
          </h2>
          <div className="space-y-1">
            {/* Currency */}
            <div className="flex items-center justify-between p-4 bg-obsidian border border-white/5">
              <div className="flex items-center gap-3">
                <Globe size={16} className="text-text-secondary" />
                <span className="font-body text-sm text-text-primary">Currency</span>
              </div>
              <div className="flex gap-2">
                {["USD", "EUR", "GBP"].map((c) => (
                  <button
                    key={c}
                    onClick={() => setCurrency(c)}
                    className={`px-2 py-1 font-mono text-xs border transition-colors ${
                      currency === c ? "border-cyan text-cyan" : "border-white/10 text-text-secondary"
                    }`}
                  >
                    {c}
                  </button>
                ))}
              </div>
            </div>

            {/* Theme */}
            <div className="flex items-center justify-between p-4 bg-obsidian border border-white/5">
              <div className="flex items-center gap-3">
                {theme === "dark" ? <Moon size={16} className="text-text-secondary" /> : <Sun size={16} className="text-text-secondary" />}
                <span className="font-body text-sm text-text-primary">Theme</span>
              </div>
              <div className="flex gap-2">
                {[
                  { value: "dark" as const, icon: Moon },
                  { value: "light" as const, icon: Sun },
                  { value: "system" as const, icon: Monitor },
                ].map((t) => (
                  <button
                    key={t.value}
                    onClick={() => setTheme(t.value)}
                    className={`p-2 border transition-colors ${
                      theme === t.value ? "border-cyan text-cyan" : "border-white/10 text-text-secondary"
                    }`}
                  >
                    <t.icon size={14} />
                  </button>
                ))}
              </div>
            </div>
          </div>
        </div>

        {/* Security */}
        <div className="mb-8">
          <h2 className="font-mono text-xs text-text-secondary uppercase tracking-wider mb-4">
            Security
          </h2>
          <div className="space-y-1">
            <Link
              to="/set-password"
              className="flex items-center justify-between p-4 bg-obsidian border border-white/5 hover:border-white/10 transition-colors"
            >
              <div className="flex items-center gap-3">
                <Lock size={16} className="text-text-secondary" />
                <span className="font-body text-sm text-text-primary">Change Password</span>
              </div>
              <ChevronRight size={16} className="text-text-tertiary" />
            </Link>

            <div className="flex items-center justify-between p-4 bg-obsidian border border-white/5">
              <div className="flex items-center gap-3">
                <Shield size={16} className="text-text-secondary" />
                <span className="font-body text-sm text-text-primary">Biometrics</span>
              </div>
              <button
                onClick={() => setBiometrics(!biometrics)}
                className={`w-10 h-5 rounded-full transition-colors relative ${
                  biometrics ? "bg-cyan" : "bg-white/10"
                }`}
              >
                <div
                  className={`w-4 h-4 rounded-full bg-white absolute top-0.5 transition-transform ${
                    biometrics ? "translate-x-5" : "translate-x-0.5"
                  }`}
                />
              </button>
            </div>

            <div className="flex items-center justify-between p-4 bg-obsidian border border-white/5">
              <div className="flex items-center gap-3">
                <Clock size={16} className="text-text-secondary" />
                <span className="font-body text-sm text-text-primary">Auto-lock</span>
              </div>
              <span className="font-mono text-xs text-text-secondary">5 minutes</span>
            </div>

            <Link
              to="/create-wallet"
              className="flex items-center justify-between p-4 bg-obsidian border border-white/5 hover:border-white/10 transition-colors"
            >
              <div className="flex items-center gap-3">
                <Shield size={16} className="text-text-secondary" />
                <span className="font-body text-sm text-text-primary">Show Recovery Phrase</span>
              </div>
              <ChevronRight size={16} className="text-text-tertiary" />
            </Link>
          </div>
        </div>

        {/* Notifications */}
        <div className="mb-8">
          <h2 className="font-mono text-xs text-text-secondary uppercase tracking-wider mb-4">
            Notifications
          </h2>
          <div className="space-y-1">
            <div className="flex items-center justify-between p-4 bg-obsidian border border-white/5">
              <div className="flex items-center gap-3">
                <Bell size={16} className="text-text-secondary" />
                <span className="font-body text-sm text-text-primary">Push Notifications</span>
              </div>
              <button
                onClick={() => setNotifications(!notifications)}
                className={`w-10 h-5 rounded-full transition-colors relative ${
                  notifications ? "bg-cyan" : "bg-white/10"
                }`}
              >
                <div
                  className={`w-4 h-4 rounded-full bg-white absolute top-0.5 transition-transform ${
                    notifications ? "translate-x-5" : "translate-x-0.5"
                  }`}
                />
              </button>
            </div>
          </div>
        </div>

        {/* Advanced */}
        <div className="mb-8">
          <h2 className="font-mono text-xs text-text-secondary uppercase tracking-wider mb-4">
            Advanced
          </h2>
          <div className="space-y-1">
            <button className="w-full flex items-center justify-between p-4 bg-obsidian border border-white/5 hover:border-white/10 transition-colors">
              <div className="flex items-center gap-3">
                <Trash2 size={16} className="text-text-secondary" />
                <span className="font-body text-sm text-text-primary">Clear Cache</span>
              </div>
              <span className="font-mono text-xs text-text-tertiary">24.5 MB</span>
            </button>

            <Link
              to="/networks"
              className="flex items-center justify-between p-4 bg-obsidian border border-white/5 hover:border-white/10 transition-colors"
            >
              <div className="flex items-center gap-3">
                <Globe size={16} className="text-text-secondary" />
                <span className="font-body text-sm text-text-primary">Networks</span>
              </div>
              <div className="flex items-center gap-2">
                <span className="font-mono text-xs text-text-secondary">zkSync Era</span>
                <ChevronRight size={16} className="text-text-tertiary" />
              </div>
            </Link>
          </div>
        </div>

        {/* Logout */}
        <button
          onClick={() => setShowLogoutConfirm(true)}
          className="w-full flex items-center justify-center gap-2 p-4 bg-red/5 border border-red/20 text-red font-mono text-sm hover:bg-red/10 transition-colors"
        >
          <LogOut size={16} />
          Logout
        </button>

        {/* Version */}
        <p className="text-center mt-8 font-mono text-[10px] text-text-tertiary">
          Nautica v2.1.0 (build 892)
        </p>

        {/* Logout Confirmation */}
        {showLogoutConfirm && (
          <div className="fixed inset-0 bg-black/70 flex items-center justify-center z-50 px-4">
            <div className="bg-obsidian border border-white/10 p-6 max-w-sm w-full">
              <h3 className="font-heading text-lg text-text-primary">Logout?</h3>
              <p className="mt-2 text-text-secondary text-sm">
                You will need your password or seed phrase to access your wallet again.
              </p>
              <div className="flex gap-3 mt-6">
                <button
                  onClick={() => setShowLogoutConfirm(false)}
                  className="flex-1 py-3 border border-white/10 text-text-secondary font-mono text-sm"
                >
                  Cancel
                </button>
                <Link
                  to="/welcome"
                  className="flex-1 py-3 bg-red text-white font-mono text-sm text-center"
                >
                  Logout
                </Link>
              </div>
            </div>
          </div>
        )}
      </div>
    </div>
  );
}
