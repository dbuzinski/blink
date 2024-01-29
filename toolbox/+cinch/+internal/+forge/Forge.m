classdef Forge
    properties
        Cache (1,1) dictionary = dictionary(string.empty(), function_handle.empty())
    end

    methods
        function result = render(obj, template, context)
            arguments
                obj
                template (1,1) string
                context = struct()
            end
            if ~isKey(obj.Cache, template)
                obj.Cache(template) = obj.compile(template);
            end
            tmplFn = obj.Cache(template);
            result = string(tmplFn(context)).replace("\n", newline).replace("\quote", """");
        end
    end

    methods (Access=private)
        function ct = compile(obj, template)
            arguments
                obj
                template (1,1) string
            end
            pt = obj.parse(template);
            ct = eval("@(c)"""+pt+"""");
        end

        function pt = parse(obj, template, stack)
            arguments
                obj
                template (1,1) string
                stack (1,1) double = 0
            end
            % Remove \r and escape quotes
            pt = template.replace(newline, "\n").replace("\r", "").replace("""", "\quote");
            % Replace all comments
            [captureGroups, matches] = regexp(pt, "(\\*){%[\s\S]*?%}", "tokens", "emptymatch", "match");
            for i=1:numel(matches)
                args = [{matches(i)} num2cell(captureGroups{i}) {pt}];
                pt = pt.replace(matches(i), replaceComments(obj, stack, args{:}));
            end
            % Replace all tags
            [captureGroups, matches, ~, ind] = regexp(pt, "(\\*){(([\w_.\-@:]+)|>([\w_.\-@:]+)|for +([^ }]+) *= *([^}]+)|if +(~ *|)([^}]+))}", "tokens", "match", "emptymatch");
            if numel(matches) > 0
                args = [{matches(1)} num2cell(captureGroups{1}) {pt}];
                if matches(1).startsWith("{for ") || matches(1).startsWith("{if ")
                    stack = stack + 1;
                elseif matches(1) == "{end}"
                    stack = stack - 1;
                end
                left = extractBefore(pt, ind(1)+1).replace(matches(1), replaceTags(obj, stack, args{:}));
                right = obj.parse(extractAfter(pt, ind(1)), stack);
                pt = left + right;
            end
        end

        function out = replaceByFunction(obj, str, pattern, replacer, stack)
            out = str;
            [captureGroups, matches, offsets] = regexp(str, pattern, "tokens", "emptymatch", "match");
            for i=1:numel(matches)
                args = [{matches(i)} num2cell(captureGroups{i}) {offsets(i)} {str}];
                out = out.replace(matches(i), replacer(obj, stack, args{:}));
            end
        end
        
        function out = replaceComments(~, ~, str, escapeChar, ~, ~)
            if strlength(escapeChar) > 0
                out = regexprep(str, "\\", "", "once");
            else
                out = "";
            end
        end
        
        function out = replaceTags(varargin)
            obj = varargin{1};
            stack = varargin{2};
            str = varargin{3};
            out = str;
            out = replaceByFunction(obj, out, "(\\*){([\w_.\-@:]+)}", @replaceVars, stack);
            out = replaceByFunction(obj, out, "(\\*){>([\w_.\-@:]+)}", @replacePartials, stack);
            out = replaceByFunction(obj, out, "(\\*){for +([\w_\-@:]+) *= *([^}]+)}", @replaceFor, stack);
            out = replaceByFunction(obj, out, "(\\*){if +(~ *|)([^}]+)}", @replaceIf, stack);
        end
        
        function out = replaceVars(~, stack, str, escapeChar, var, ~, ~)
            out = str;
            if stack > 0
                return
            end
            if strlength(escapeChar) > 0
                out = regexprep(str, "\\", "", "once");
            elseif strlength(var) > 0
                if var == "end"
                    out = """)+""";
                else
                    out = """+c."+var+"+""";
                end
            end
        end
        
        function out = replacePartials(~, stack, str, escapeChar, partial, ~, ~)
            out = str;
            if stack > 0
                return
            end
            if strlength(escapeChar) > 0
                out = regexprep(str, "\\", "", "once");
            elseif strlength(partial) > 0
                out = """+obj.render(c."+partial+",c)+""";
            end
        end
        
        function out = replaceIf(~, stack, str, escapeChar, ifNot, ifKey, ~, ~)
            out = str;
            if stack > 1
                return
            end
            if strlength(escapeChar) > 0
                out = regexprep(str, "\\", "", "once");
            elseif strlength(ifKey) > 0
                if strlength(ifNot) > 0
                    neg = "~";
                else
                    neg = "";
                end
                out = """+ifReplacement(obj,c,"""+ifKey+""","""+neg+""",""";
            end
        end
        
        function fstr = ifReplacement(obj, c, ifKey, neg, template)
            if ~isempty(fieldnames(c))
                for f = string(fieldnames(c))'
                    eval(f+"=c.(f);");
                end
            end
            fstr = "";
            safeIfKey = eval(neg+"("+ifKey+")");
            if safeIfKey
                fstr=fstr + obj.render(template,c);
            end
        end
        
        function out = replaceFor(~, stack, str, escapeChar, iterVar, forKey, ~, ~)
            out = str;
            if stack > 1
                return
            end
            if strlength(escapeChar) > 0
                out = regexprep(str, "\\", "", "once");
            elseif strlength(forKey) > 0
                out = """+forReplacement(obj,c,"""+iterVar+""","""+forKey+""",""";
            end
        end

        function fstr = forReplacement(obj, c,iterVar, forKey, template)
                if ~isempty(fieldnames(c))
                    for f = string(fieldnames(c))'
                        eval(f+"=c.(f);");
                    end
                end
                fstr = "";
                safeForKey = eval(forKey);
                safeForKey = safeForKey(:)';
                for safeIterVar = safeForKey
                    c.(iterVar)=safeIterVar;
                    fstr = fstr + obj.render(template,c);
                end
        end
    end
end