/**
 * zap - App frame(?)work base.
 * ZIK@MMXXVI
 */

#ifndef __ZAP_GUARD
#define __ZAP_GUARD

namespace zap {
    /** App lifecycle abstraction. */
    class App {

    protected:
        /** Startup /  begin the app. */
        virtual void boot() = 0;
        /** Shut down the app. */
        virtual void shutdown() = 0;

        // Hooks for app lifecycle
        /** Called on app initialization phase. */
        virtual void on_setup() = 0;
    };
}
#endif