import { useTheme } from "@/hooks/use-theme";

export function ThemeToggle({ compact = false }: { compact?: boolean }) {
  const { isDark, toggle } = useTheme();

  return (
    <button
      onClick={toggle}
      aria-label="Toggle theme"
      className={
        "group relative inline-flex items-center gap-2 rounded-full border border-border transition-all duration-300 hover:border-primary/30 " +
        (compact
          ? "w-10 h-10 justify-center bg-muted"
          : "px-4 py-2.5 text-xs font-medium bg-muted")
      }
    >
      {/* Sun/Moon animated toggle */}
      <div className="relative w-5 h-5">
        {/* Sun */}
        <svg
          className={
            "absolute inset-0 w-5 h-5 transition-all duration-500 " +
            (isDark
              ? "opacity-0 rotate-90 scale-0"
              : "opacity-100 rotate-0 scale-100")
          }
          viewBox="0 0 24 24"
          fill="none"
          stroke="currentColor"
          strokeWidth={2}
          strokeLinecap="round"
        >
          <circle cx="12" cy="12" r="5" />
          <line x1="12" y1="1" x2="12" y2="3" />
          <line x1="12" y1="21" x2="12" y2="23" />
          <line x1="4.22" y1="4.22" x2="5.64" y2="5.64" />
          <line x1="18.36" y1="18.36" x2="19.78" y2="19.78" />
          <line x1="1" y1="12" x2="3" y2="12" />
          <line x1="21" y1="12" x2="23" y2="12" />
          <line x1="4.22" y1="19.78" x2="5.64" y2="18.36" />
          <line x1="18.36" y1="5.64" x2="19.78" y2="4.22" />
        </svg>
        {/* Moon */}
        <svg
          className={
            "absolute inset-0 w-5 h-5 transition-all duration-500 " +
            (isDark
              ? "opacity-100 rotate-0 scale-100"
              : "opacity-0 -rotate-90 scale-0")
          }
          viewBox="0 0 24 24"
          fill="none"
          stroke="currentColor"
          strokeWidth={2}
          strokeLinecap="round"
        >
          <path d="M21 12.79A9 9 0 1 1 11.21 3 7 7 0 0 0 21 12.79z" />
        </svg>
      </div>
      {!compact && (
        <span className="text-foreground font-display">
          {isDark ? "Dark" : "Light"}
        </span>
      )}
    </button>
  );
}
