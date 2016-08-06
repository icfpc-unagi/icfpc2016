package tc.wata.debug;

/**
 * 終了時にまとめて出力されるログ
 */
public class Log {
	
	public static TextColor[] color = {new TextColor().color(TextColor.BLUE).bold(), new TextColor().color(TextColor.YELLOW).bold(), new TextColor().color(TextColor.RED).bold()};
	private static StringBuilder log;
	
	private static void setup() {
		log = new StringBuilder("----- Error Log -----");
		Runtime.getRuntime().addShutdownHook(new Thread() {
			@Override
			public void run() {
				System.err.println(log);
			}
		});
	}
	
	/**
	 * ログに追加する(呼び出した段階でエラー出力に表示され，プログラム終了時にもまとめて出力される)
	 * @param level 0 - 2
	 */
	public static void add(int level, String text) {
		if (log == null) setup();
		text = color[level].apply(text);
		System.err.println(text);
		log.append(String.format("%n")).append(text);
	}
	
}
