classdef Response
    properties
        Data (1,:) char
        StatusCode (1,1) int16 = 200
    end

    methods
        function resp = Response(varargin)
        end
        
        function resp = render(resp, renderer, template, context, partials)
            arguments
                resp
                renderer
                template (1,1) string
                context (1,1) struct = struct()
                partials (1,1) struct = struct()
            end
            resp.Data = renderer.render(template, context, partials);
        end
    end
end
