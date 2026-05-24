import type { ReactNode } from "react";
import { TopBar, SideRail, BottomDock } from "./Shell";

export function AppLayout({ children }: { children: ReactNode }) {
  return (
    <div className="min-h-screen bg-background">
      <div className="flex">
        <SideRail />
        <main className="flex-1 min-w-0 pb-28 md:pb-10">
          <TopBar />
          <div className="px-5 md:px-8 lg:px-12 pt-2 md:pt-8 max-w-[1200px] mx-auto">
            {children}
          </div>
        </main>
      </div>
      <BottomDock />
    </div>
  );
}
