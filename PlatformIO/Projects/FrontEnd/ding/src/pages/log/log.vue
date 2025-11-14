<template>
	<view class="container">
		<button @click="getAliveEspConnect()" class="btn">获取连接</button>
		<uni-popup ref="popupRef" background-color="#fff" @change="change">
			<view class="popup-content" :class="{ 'popup-height': type === 'left' || type === 'right' }">
				<uni-table ref="tableRef" border stripe type="selection" emptyText="暂无更多数据" @selection-change="selectionChange">
					<uni-tr>
						<uni-th width="80" align="center">ID</uni-th>
						<uni-th width="center" align="center">alive</uni-th>
						<uni-th>ready</uni-th>
					</uni-tr>
					<uni-tr v-for="(item, index) in tableData" :key="index">
						<uni-td>{{ item.deviceId }}</uni-td>
						<uni-td>
							<view class="name">{{ item.alive }}</view>
						</uni-td>
						<uni-td align="center">{{ item.readyState }}</uni-td>
					</uni-tr>
				</uni-table>
			</view>
		</uni-popup>
		<view class="log-container">
			<uni-table ref="logTableRef" border stripe type="selection" emptyText="暂无更多数据">
				<uni-tr>
					<uni-th align="center">类型</uni-th>
					<uni-th align="center">ID</uni-th>
					<uni-th align="center">动作</uni-th>
					<uni-th align="center">结果</uni-th>
					<uni-th align="center">消息</uni-th>
					<uni-th>时间</uni-th>
				</uni-tr>
				<uni-tr v-for="(item, index) in logTableData" :key="index">
					<uni-td>{{ item.type }}</uni-td>
					<uni-td>{{ item.deviceId }}</uni-td>
					<uni-td>
						<view class="name">{{ item.action }}</view>
					</uni-td>
					<uni-td align="center">{{ item.result }}</uni-td>
					<uni-td align="center">{{ item.message }}</uni-td>
					<uni-td align="center">{{ item.time }}</uni-td>
				</uni-tr>
			</uni-table>
		</view>
	</view>
</template>

<script setup lang="ts">
import { ref, onMounted, onBeforeUnmount, nextTick } from 'vue';
import { getWebSocketInstance } from '@/utils/getSocketIns';
import req from '@/api/index';
const EspListTable = ref();
const popupRef = ref();
const tableData = ref([]);
const logTableData = ref([]);
const type = ref('right');
const ws = getWebSocketInstance();
const formData = ref({
	name: 'akue'
});
// 注册函数
const registClient = () => {
	const token = uni.getStorageSync('token')
	if (ws.socketOpen) {
		ws.send({ type: 'register', clientId: 'browser_001',token:token });
	} else {
		setTimeout(registClient, 500);
	}
};

// 接收消息
ws.onMessage((data) => {
	let msg;
	// 解析websocket信息
	try {
		msg = typeof data === 'string' ? JSON.parse(data) : data;
	} catch (e) {
		console.warn('收到非 JSON 消息', data);
		return;
	}
	// 往msg对象中添加属性
	msg.time = formatTs(msg.ts);
	// 把websocket信息放在表格中
	console.log('msg', msg);
	logTableData.value.unshift(msg);
});

// 格式化时间戳
const formatTs = (ts) => {
	const date = new Date(ts);
	const Y = date.getFullYear();
	const M = String(date.getMonth() + 1).padStart(2, '0');
	const D = String(date.getDate()).padStart(2, '0');
	const h = String(date.getHours()).padStart(2, '0');
	const m = String(date.getMinutes()).padStart(2, '0');
	const s = String(date.getSeconds()).padStart(2, '0');
	return `${Y}-${M}-${D} ${h}:${m}:${s}`;
};

// 获取当前EspMap中连接数量以及ID
const getAliveEspConnect = () => {
	req.getEspAliveConnect()
		.then((res) => {
			tableData.value = res.devices;
			toggle('right');
		})
		.catch((e) => {
			console.log(e);
		});
};

//
const selectionChange = (e) => {
	console.log(e.detail.index);
};
const toggle = (type: String) => {
	popupRef.value.open(type);
};
const change = (e) => {
	// console.log('当前模式：' + e.type + ',状态：' + e.show);
};
onMounted(() => {
	registClient();
});
</script>
<style scoped lang="scss">
.container {
	flex: 1;
	padding: 10px;
}

@mixin height {
	/* #ifndef APP-NVUE */
	height: 100%;
	/* #endif */
	/* #ifdef APP-NVUE */
	flex: 1;
	/* #endif */
}
.popup-height {
	@include height;
	width: 300px;
}

.uni-group {
	display: flex;
	align-items: center;
}

.log-container {
	height: 100%;
}

.log-item {
	display: flex;
	flex-direction: row;
	justify-content: space-between;
	flex-wrap: wrap; /* 允许换行 */
	padding: 5px 0;
	border-bottom: 1px solid #eee;
}

.log-text {
	flex: 1 1 auto; /* 文本自适应宽度 */
	word-break: break-word; /* 换行显示 */
}

.log-time {
	flex: 0 0 auto;
	margin-left: 5px;
	color: #999;
	font-size: 12px;
}
</style>
