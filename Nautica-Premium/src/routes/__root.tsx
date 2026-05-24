import { QueryClient, QueryClientProvider } from "@tanstack/react-query";
import {
  Outlet,
  Link,
  createRootRouteWithContext,
  useRouter,
  HeadContent,
  Scripts,
} from "@tanstack/react-router";
import { ThemeProvider } from "@/hooks/use-theme";

import appCss from "../styles.css?url";

function NotFoundComponent() {
  return (
    <div className="flex min-h-screen items-center justify-center bg-background px-4">
      <div className="max-w-md text-center animate-fade-up">
        <div className="w-20 h-20 mx-auto mb-6 rounded-3xl bg-muted grid place-items-center">
          <span className="text-4xl">🌊</span>
        </div>
        <h1 className="font-display text-6xl font-bold text-ink">404</h1>
        <h2 className="mt-3 text-lg font-display font-semibold text-foreground">
          Lost at sea
        </h2>
        <p className="mt-2 text-sm text-muted-foreground">
          The page you're looking for doesn't exist or has drifted away.
        </p>
        <div className="mt-6">
          <Link to="/" className="btn-primary">
            Navigate home
          </Link>
        </div>
      </div>
    </div>
  );
}

function ErrorComponent({
  error,
  reset,
}: {
  error: Error;
  reset: () => void;
}) {
  console.error(error);
  const router = useRouter();

  return (
    <div className="flex min-h-screen items-center justify-center bg-background px-4">
      <div className="max-w-md text-center animate-fade-up">
        <div className="w-20 h-20 mx-auto mb-6 rounded-3xl bg-destructive/10 grid place-items-center">
          <span className="text-4xl">⚠️</span>
        </div>
        <h1 className="text-xl font-display font-bold text-ink">
          Something went wrong
        </h1>
        <p className="mt-2 text-sm text-muted-foreground">
          We hit a reef. You can try again or head back to port.
        </p>
        <div className="mt-6 flex flex-wrap justify-center gap-3">
          <button
            onClick={() => {
              router.invalidate();
              reset();
            }}
            className="btn-primary"
          >
            Try again
          </button>
          <a href="/" className="btn-secondary">
            Go home
          </a>
        </div>
      </div>
    </div>
  );
}

export const Route = createRootRouteWithContext<{
  queryClient: QueryClient;
}>()(
  {
    head: () => ({
      meta: [
        { charSet: "utf-8" },
        { name: "viewport", content: "width=device-width, initial-scale=1" },
        {
          title: "Nautica — Premium NFT Marketplace",
        },
        {
          name: "description",
          content:
            "Discover, collect, create and trade on-chain art. Send private, ZK-proven payments. The marketplace built for serious collectors.",
        },
        { name: "author", content: "Nautica" },
        {
          property: "og:title",
          content: "Nautica — Premium NFT Marketplace",
        },
        {
          property: "og:description",
          content:
            "Discover, collect, and trade on-chain art with zero-knowledge privacy.",
        },
        { property: "og:type", content: "website" },
        { name: "twitter:card", content: "summary_large_image" },
        { name: "theme-color", content: "#0a0f1e" },
      ],
      links: [
        { rel: "stylesheet", href: appCss },
        { rel: "icon", type: "image/png", href: "/favicon.png" },
        { rel: "apple-touch-icon", href: "/favicon.png" },
        { rel: "preconnect", href: "https://fonts.googleapis.com" },
        {
          rel: "preconnect",
          href: "https://fonts.gstatic.com",
          crossOrigin: "anonymous",
        },
        {
          rel: "stylesheet",
          href: "https://fonts.googleapis.com/css2?family=Outfit:wght@300;400;500;600;700;800&family=Inter:wght@300;400;500;600;700&family=Instrument+Serif:ital@0;1&display=swap",
        },
      ],
    }),
    shellComponent: RootShell,
    component: RootComponent,
    notFoundComponent: NotFoundComponent,
    errorComponent: ErrorComponent,
  },
);

function RootShell({ children }: { children: React.ReactNode }) {
  return (
    <html lang="en" className="no-transition">
      <head>
        <HeadContent />
      </head>
      <body>
        <script
          dangerouslySetInnerHTML={{
            __html: `(function(){try{var t=localStorage.getItem('nautica.theme');if(t==='light')document.documentElement.classList.add('light');document.documentElement.classList.remove('no-transition');requestAnimationFrame(function(){requestAnimationFrame(function(){document.documentElement.classList.remove('no-transition')})});}catch(e){}})();`,
          }}
        />
        {children}
        <Scripts />
      </body>
    </html>
  );
}

function RootComponent() {
  const { queryClient } = Route.useRouteContext();

  return (
    <QueryClientProvider client={queryClient}>
      <ThemeProvider>
        <Outlet />
      </ThemeProvider>
    </QueryClientProvider>
  );
}
