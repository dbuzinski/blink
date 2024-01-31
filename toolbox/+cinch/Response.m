classdef Response
    properties
        Data (1,:) char
        StatusCode (1,1) int16 = 200
    end

    methods
        function resp = Response(varargin)
        end
        
        function resp = render(resp, template, context)
            arguments
                resp
                template (1,1) string
                context (1,1) struct = struct()
            end
            import cinch.internal.forge.Forge;

            f = Forge();
            tpl = string(fileread(template));
            resp.Data = f.render(tpl, context);
        end
    end
end