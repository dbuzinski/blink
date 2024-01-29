classdef App < handle
    properties
        Routes (1,:) cinch.Route = cinch.Route.empty()
        StaticFiles (1,:) char
        StaticRoute (1,:) char = '/static'
    end

    methods
        function serve(app, options)
            arguments
                app
                options.Port = 5000
            end
            if ~isempty(app.StaticFiles)
                cinch.internal.serve(options.Port, app.Routes, app.StaticFiles, app.StaticRoute);
            else
                cinch.internal.serve(options.Port, app.Routes);
            end
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