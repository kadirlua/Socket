package com.sdk.socket;

public class Utils {
    public static boolean isValidIPAddressDomain(String input) {
        String ipPattern =
                "^((25[0-5]|2[0-4][0-9]|[0-1]?[0-9][0-9]?)\\.){3}" +
                        "(25[0-5]|2[0-4][0-9]|[0-1]?[0-9][0-9]?)" +
                        "(\\:[0-9]{1,5})?$";

        String domainPattern =
                "^(?!-)[A-Za-z0-9-]{1,63}(?<!-)\\." +
                        "[A-Za-z]{2,6}" +
                        "(\\:[0-9]{1,5})?$";

        return input.matches(ipPattern) || input.matches(domainPattern);
    }

    public static boolean isValidPort(String input) {
        String portPattern = "([0-9]{1,5})";
        return input.matches(portPattern);
    }
}
