classdef App < handle
    properties
        Routes (1,:) blink.Route = blink.Route.empty()
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
                blink.internal.serve(options.Port, app.Routes, app.StaticFiles, app.StaticRoute);
            else
                blink.internal.serve(options.Port, app.Routes);
            end
        end

        function app = get(app, path, handler)
            route = blink.Route("GET", path, handler);
            app.Routes = [app.Routes route];
        end

        function app = post(app, path, handler)
            route = blink.Route("POST", path, handler);
            app.Routes = [app.Routes route];
        end

        function app = put(app, path, handler)
            route = blink.Route("PUT", path, handler);
            app.Routes = [app.Routes route];
        end

        function app = delete(app, path, handler)
            route = blink.Route("DELETE", path, handler);
            app.Routes = [app.Routes route];
        end
    end
end
