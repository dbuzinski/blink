classdef tResponse < matlab.unittest.TestCase
    methods(Test)
        function testResponseCreation(testCase)
            resp = blink.Response();
            
            testCase.verifyEqual(resp.StatusCode, int16(200));
            testCase.verifyEqual(resp.Data, char.empty(1, 0));
            testCase.verifyEqual(resp.ContentType, "");
            testCase.verifyTrue(isempty(fieldnames(resp.Headers)));
        end
    end
end
