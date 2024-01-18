classdef App < handle
    properties
        Config (1,1) cinch.Config
        Routes (1,:) cinch.Route = cinch.Route.empty()
    end

    methods
        function serve(app, options)
            arguments
                app
                options.Port = 5000
            end
            cinch.internal.serve(options.Port, app.Routes);
        end

        function app = get(app, path, handler)
            route = cinch.Route("GET", path, handler);
            app.Routes = [app.Routes route];
        end

        function app = post(app, path, handler)
            route = cinch.Route("POST", path, handler);
            app.Routes = [app.Routes route];
        end

        function app = put(app, path, handler)
            route = cinch.Route("PUT", path, handler);
            app.Routes = [app.Routes route];
        end

        function app = delete(app, path, handler)
            route = cinch.Route("DELETE", path, handler);
            app.Routes = [app.Routes route];
        end
    end
end